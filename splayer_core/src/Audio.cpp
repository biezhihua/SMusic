
#include <Audio.h>

#include "Audio.h"

Audio::Audio() = default;

Audio::~Audio() = default;

void Audio::setStream(Stream *stream) {
    Audio::stream = stream;
}

int Audio::openAudio(int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate,
                     AudioParams *audio_hw_params) {
    return 0;
}

void Audio::pauseAudio() {

}

/**
 * Decode one audio frame and return its uncompressed size.
 *
 * The processed audio frame is decoded, converted if required, and
 * stored in is->audio_buf, with size in bytes given by the return
 * value.
 */
int Audio::audioDecodeFrame() {
    VideoState *is = Audio::stream->getVideoState();

    if (!is) {
        ALOGD(AUDIO_TAG, "%s video state is null", __func__);
        return NEGATIVE(S_NULL);
    }

    int reSampledDataSize;
    int64_t wantedChannelLayout;
    double audioClock;
    int wantedNbSamples;
    Frame *frame;

    if (is->paused) {
        ALOGD(AUDIO_TAG, "%s paused ", __func__);
        return NEGATIVE(S_AUDIO_PAUSED);
    }

    do {
        if (!(frame = is->audioFrameQueue.peekReadable())) {
            ALOGD(AUDIO_TAG, "%s not readable ", __func__);
            return NEGATIVE(S_NOT_FRAME_READABLE);
        }
        is->audioFrameQueue.next();
    } while (frame->seekSerial != is->audioPacketQueue.seekSerial);

    wantedChannelLayout = getWantedChannelLayout(frame);

    wantedNbSamples = synchronizeAudio(frame->frame->nb_samples);

    bool isNotSameSampleFormat = (AVSampleFormat) frame->frame->format != is->audioSrc.sampleFormat;
    bool isNotSameChannelLayout = wantedChannelLayout != is->audioSrc.channelLayout;
    bool isNotSameSampleRate = frame->frame->sample_rate != is->audioSrc.sampleRate;
    bool isNotSameNbSamples = wantedNbSamples != frame->frame->nb_samples && !is->audioSwrContext;
    bool isNeedConvert = isNotSameSampleFormat || isNotSameChannelLayout || isNotSameSampleRate || isNotSameNbSamples;

    if (isNeedConvert) {
        if (IS_NEGATIVE(initConvertSwrContext(is, wantedChannelLayout, frame))) {
            return NEGATIVE(S_NOT_INIT_CONVERSION_CONTEXT);
        }
    }

    if (is->audioSwrContext) {
        int length = convertAudio(is, wantedNbSamples, frame);
        if (length > 0) {
            is->audioBuf = is->audioConvertBuf;
            reSampledDataSize =
                    length * is->audioTarget.channels * av_get_bytes_per_sample(is->audioTarget.sampleFormat);
        } else {
            return NEGATIVE(S_NOT_CONVERT_AUDIO);
        }
    } else {
        is->audioBuf = frame->frame->data[0];
        reSampledDataSize = av_samples_get_buffer_size(nullptr, frame->frame->channels,
                                                       frame->frame->nb_samples,
                                                       (AVSampleFormat) frame->frame->format,
                                                       1);;
    }

    audioClock = is->audioClockTime;

    /* update the audio clock with the pts */
    if (!isnan(frame->pts)) {
        is->audioClockTime = frame->pts + (double) frame->frame->nb_samples / frame->frame->sample_rate;
    } else {
        is->audioClockTime = NAN;
    }

    is->audioClockSerial = frame->seekSerial;

    ALOGD(AUDIO_TAG, "%s audio: delay=%0.3f clock=%0.3f clock0=%0.3f", __func__,
          is->audioClockTime - is->lastAudioClockTime,
          is->audioClockTime,
          audioClock);

    is->lastAudioClockTime = is->audioClockTime;

    return reSampledDataSize;
}

int Audio::convertAudio(VideoState *is, int wantedNbSamples, Frame *frame) {

    const uint8_t **in = (const uint8_t **) frame->frame->extended_data;
    uint8_t **out = &is->audioConvertBuf;

    int outCount = static_cast<int>((int64_t) wantedNbSamples * is->audioTarget.sampleRate / frame->frame->sample_rate +
                                    256);

    int outSize = av_samples_get_buffer_size(nullptr,
                                             is->audioTarget.channels,
                                             outCount,
                                             is->audioTarget.sampleFormat,
                                             0);
    int length;

    if (outSize < 0) {
        ALOGE(AUDIO_TAG, "%s av_samples_get_buffer_size() failed", __func__);
        return NEGATIVE(S_AUDIO_OUT_SIZE);
    }

    if (wantedNbSamples != frame->frame->nb_samples) {
        if (swr_set_compensation(is->audioSwrContext,
                                 (wantedNbSamples - frame->frame->nb_samples) * is->audioTarget.sampleRate /
                                 frame->frame->sample_rate,
                                 wantedNbSamples * is->audioTarget.sampleRate / frame->frame->sample_rate) < 0) {
            ALOGE(AUDIO_TAG, "%s swr_set_compensation() failed", __func__);
            return NEGATIVE(S_NOT_AUDIO_SWR_COMPENSATION);
        }
    }

    av_fast_malloc(&is->audioConvertBuf, &is->audioBuf1Size, static_cast<size_t>(outSize));

    if (!is->audioConvertBuf) {
        return NEGATIVE(S_NOT_MEMORY);
    }

    length = swr_convert(is->audioSwrContext, out, outCount, in, frame->frame->nb_samples);

    if (length < 0) {
        ALOGE(AUDIO_TAG, "%s swr_convert() failed", __func__);
        return NEGATIVE(S_NOT_CONVERT_AUDIO);
    }

    if (length == outCount) {
        ALOGD(AUDIO_TAG, "%s audio buffer is probably too small", __func__);
        if (swr_init(is->audioSwrContext) < 0) {
            swr_free(&is->audioSwrContext);
        }
    }
    return length;
}

int Audio::initConvertSwrContext(VideoState *is, int64_t desireChannelLayout, const Frame *frame) const {

    swr_free(&is->audioSwrContext);

    is->audioSwrContext = swr_alloc_set_opts(nullptr,
                                             is->audioTarget.channelLayout,
                                             is->audioTarget.sampleFormat,
                                             is->audioTarget.sampleRate,
                                             desireChannelLayout,
                                             (AVSampleFormat) frame->frame->format,
                                             frame->frame->sample_rate,
                                             0, nullptr);

    if (!is->audioSwrContext || swr_init(is->audioSwrContext) < 0) {
        ALOGE(AUDIO_TAG,
              "%s Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!",
              __func__,
              frame->frame->sample_rate,
              av_get_sample_fmt_name((AVSampleFormat) frame->frame->format),
              frame->frame->channels,
              is->audioTarget.sampleRate,
              av_get_sample_fmt_name(is->audioTarget.sampleFormat),
              is->audioTarget.channels);

        swr_free(&is->audioSwrContext);

        if (msgQueue) {
            msgQueue->notifyMsg(Msg::MSG_ERROR, S_NOT_CREATE_AUDIO_SWR_CONTEXT);
        }

        return NEGATIVE(S_NOT_CREATE_AUDIO_SWR_CONTEXT);
    }

    is->audioSrc.channelLayout = desireChannelLayout;
    is->audioSrc.channels = frame->frame->channels;
    is->audioSrc.sampleRate = frame->frame->sample_rate;
    is->audioSrc.sampleFormat = (AVSampleFormat) frame->frame->format;

    ALOGD(AUDIO_TAG, "%s channelLayout = %lld channels = %d sampleRate = %d sampleFormat = %d", __func__,
          is->audioSrc.channelLayout,
          is->audioSrc.channels,
          is->audioSrc.sampleRate,
          is->audioSrc.sampleFormat
    );

    return POSITIVE;
}

uint64_t Audio::getWantedChannelLayout(const Frame *frame) const {
    bool isValid = frame->frame->channel_layout &&
                   frame->frame->channels == av_get_channel_layout_nb_channels(frame->frame->channel_layout);
    return isValid ? frame->frame->channel_layout : av_get_default_channel_layout(frame->frame->channels);
}

/* return the wanted number of samples to get better sync if sync_type is video
 * or external master clock */
int Audio::synchronizeAudio(int nbSamples) {

    //TODO:

    VideoState *is = stream->getVideoState();
    int wantedNbSamples = nbSamples;

    /* if not master, then we try to remove or add samples to correct the clock */
    if (stream->getMasterSyncType() != SYNC_TYPE_AUDIO_MASTER) {
        double diff, avgDiff;
        int minNbSamples, maxNbSamples;

        diff = is->audioClock.getClock() - stream->getMasterClock();

        bool isNotNan = !isnan(diff);
        bool isNeedSync = fabs(diff) < NO_SYNC_THRESHOLD;
        if (isNotNan && isNeedSync) {

            is->audioDiffCum = diff + is->audioDiffAvgCoef * is->audioDiffCum;

            if (is->audioDiffAvgCount < AUDIO_DIFF_AVG_NB) {
                /* not enough measures to have a correct estimate */
                is->audioDiffAvgCount++;
            } else {
                /* estimate the A-V difference */
                avgDiff = is->audioDiffCum * (1.0 - is->audioDiffAvgCoef);

                if (fabs(avgDiff) >= is->audioDiffThreshold) {
                    wantedNbSamples = nbSamples + (int) (diff * is->audioSrc.sampleRate);
                    minNbSamples = ((nbSamples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    maxNbSamples = ((nbSamples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    wantedNbSamples = av_clip(wantedNbSamples, minNbSamples, maxNbSamples);
                }
                ALOGD(AUDIO_TAG, "%s diff=%f adiff=%f sample_diff=%d apts=%0.3f %f", __func__, diff, avgDiff,
                      wantedNbSamples - nbSamples, is->audioClockTime, is->audioDiffThreshold);
            }
        } else {
            /* too big difference : may be initial PTS errors, so
               reset A-V filter */
            is->audioDiffAvgCount = 0;
            is->audioDiffCum = 0;
        }
    }
    return wantedNbSamples;
}

void Audio::setOptions(Options *options) {
    Audio::options = options;
}

void Audio::setMsgQueue(MessageQueue *msgQueue) {
    Audio::msgQueue = msgQueue;
}

void Audio::setMediaPlayer(MediaPlayer *mediaPlayer) {
    Audio::mediaPlayer = mediaPlayer;
}

void Audio::audioCallback(uint8_t *stream, int streamLength) {

    if (!Audio::stream) {
        ALOGE(AUDIO_TAG, "%s stream is null", __func__);
        return;
    }

    if (!Audio::stream->getVideoState()) {
        ALOGE(AUDIO_TAG, "%s video state is null", __func__);
        return;
    }

    VideoState *is = Audio::stream->getVideoState();
    int audioSize, length;

    audioCallbackTime = av_gettime_relative();

    while (streamLength > 0) {
        if (is->audioBufIndex >= is->audioBufSize) {
            audioSize = audioDecodeFrame();
            if (audioSize < 0) {
                /* if error, just output silence */
                is->audioBuf = nullptr;
                is->audioBufSize = (unsigned int) (SDL_AUDIO_MIN_BUFFER_SIZE / is->audioTarget.frameSize *
                                                   is->audioTarget.frameSize);
            } else {
                // TODO
                if (is->showMode != SHOW_MODE_VIDEO) {
                    update_sample_display((int16_t *) is->audioBuf, audioSize);
                }
                is->audioBufSize = (unsigned int) (audioSize);
            }
            is->audioBufIndex = 0;
        }

        length = is->audioBufSize - is->audioBufIndex;

        if (length > streamLength) {
            length = streamLength;
        }

        if (!is->audioMuted && is->audioBuf && is->audioVolume == MIX_MAX_VOLUME) {
            memcpy(stream, is->audioBuf + is->audioBufIndex, (size_t) (length));
        } else {
            memset(stream, 0, (size_t) length);
            if (!is->audioMuted && is->audioBuf) {
                maxAudio(stream, is->audioBuf, is->audioBufIndex, length, is->audioVolume);
            }
        }
        streamLength -= length;
        stream += length;
        is->audioBufIndex += length;
    }
    is->audioWriteBufSize = is->audioBufSize - is->audioBufIndex;
    /* Let's assume the audio driver that is used by SDL has two periods. */
    if (!isnan(is->audioClockTime)) {
        double pts = is->audioClockTime -
                     (double) (2 * is->audioHwBufSize + is->audioWriteBufSize) / is->audioTarget.bytesPerSec;
        int serial = is->audioClockSerial;
        double time = audioCallbackTime / 1000000.0;
        is->audioClock.setClockAt(pts, serial, time);
        is->exitClock.syncClockToSlave(&is->audioClock);
    }
}

void Audio::update_sample_display(short *samples, int samples_size) {

}

void Audio::maxAudio(uint8_t *stream, uint8_t *buf, int index, int length, int volume) {

}
