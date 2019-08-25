
#include <Audio.h>

#include "Audio.h"

Audio::Audio() = default;

Audio::~Audio() = default;

void Audio::setStream(Stream *stream) {
    Audio::stream = stream;
}

int Audio::openAudio(int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, AudioParams *audio_hw_params) {
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
    int dataSize, resampled_data_size;
    int64_t decChannelLayout;
    av_unused double audio_clock0;
    int wantedNbSamples;
    Frame *frame;

    if (is->paused)
        return NEGATIVE(S_AUDIO_PAUSED);

    do {
        if (!(frame = is->audioFrameQueue.peekReadable())) {
            return NEGATIVE(S_NOT_FRAME_READABLE);
        }
        is->audioFrameQueue.next();
    } while (frame->serial != is->audioPacketQueue.serial);

    dataSize = av_samples_get_buffer_size(nullptr, frame->frame->channels, frame->frame->nb_samples, (AVSampleFormat) frame->frame->format, 1);

    decChannelLayout = (frame->frame->channel_layout && frame->frame->channels == av_get_channel_layout_nb_channels(frame->frame->channel_layout)) ? frame->frame->channel_layout : av_get_default_channel_layout(frame->frame->channels);
    wantedNbSamples = synchronizeAudio(frame->frame->nb_samples);

    if (((AVSampleFormat) frame->frame->format != is->audioSrc.fmt) ||
        decChannelLayout != is->audioSrc.channel_layout ||
        frame->frame->sample_rate != is->audioSrc.freq ||
        (wantedNbSamples != frame->frame->nb_samples && !is->audioSwrContext)) {

        swr_free(&is->audioSwrContext);

        is->audioSwrContext = swr_alloc_set_opts(nullptr,
                                                 is->audioTarget.channel_layout, is->audioTarget.fmt, is->audioTarget.freq,
                                                 decChannelLayout, (AVSampleFormat) frame->frame->format, frame->frame->sample_rate,
                                                 0, nullptr);

        if (!is->audioSwrContext || swr_init(is->audioSwrContext) < 0) {
            ALOGE(AUDIO_TAG, "%s Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!", __func__,
                  frame->frame->sample_rate, av_get_sample_fmt_name((AVSampleFormat) frame->frame->format), frame->frame->channels,
                  is->audioTarget.freq, av_get_sample_fmt_name(is->audioTarget.fmt), is->audioTarget.channels);
            swr_free(&is->audioSwrContext);
            return NEGATIVE(S_NOT_AUDIO_SWR_CONTEXT);
        }
        is->audioSrc.channel_layout = decChannelLayout;
        is->audioSrc.channels = frame->frame->channels;
        is->audioSrc.freq = frame->frame->sample_rate;
        is->audioSrc.fmt = (AVSampleFormat) frame->frame->format;
    }

    if (is->audioSwrContext) {
        const uint8_t **in = (const uint8_t **) frame->frame->extended_data;
        uint8_t **out = &is->audioBuf1;
        int out_count = (int64_t) wantedNbSamples * is->audioTarget.freq / frame->frame->sample_rate + 256;
        int out_size = av_samples_get_buffer_size(nullptr, is->audioTarget.channels, out_count, is->audioTarget.fmt, 0);
        int len2;
        if (out_size < 0) {
            ALOGE(AUDIO_TAG, "%s av_samples_get_buffer_size() failed", __func__);
            return NEGATIVE(S_AUDIO_OUT_SIZE);
        }
        if (wantedNbSamples != frame->frame->nb_samples) {
            if (swr_set_compensation(is->audioSwrContext, (wantedNbSamples - frame->frame->nb_samples) * is->audioTarget.freq / frame->frame->sample_rate,
                                     wantedNbSamples * is->audioTarget.freq / frame->frame->sample_rate) < 0) {
                ALOGE(AUDIO_TAG, "%s swr_set_compensation() failed", __func__);
                return NEGATIVE(S_AUDIO_SWR_COMPENSATION);
            }
        }
        av_fast_malloc(&is->audioBuf1, &is->audioBuf1Size, out_size);
        if (!is->audioBuf1) {
            return NEGATIVE(S_NO_MEMORY);
        }
        len2 = swr_convert(is->audioSwrContext, out, out_count, in, frame->frame->nb_samples);
        if (len2 < 0) {
            ALOGE(AUDIO_TAG, "%s swr_convert() failed", __func__);
            return NEGATIVE(S_AUDIO_SWR_CONVERT_FAILED);
        }
        if (len2 == out_count) {
            ALOGD(AUDIO_TAG, "%s audio buffer is probably too small", __func__);
            if (swr_init(is->audioSwrContext) < 0) {
                swr_free(&is->audioSwrContext);
            }
        }
        is->audioBuf = is->audioBuf1;
        resampled_data_size = len2 * is->audioTarget.channels * av_get_bytes_per_sample(is->audioTarget.fmt);
    } else {
        is->audioBuf = frame->frame->data[0];
        resampled_data_size = dataSize;
    }

    audio_clock0 = is->audioClockTime;
    /* update the audio clock with the pts */
    if (!isnan(frame->pts)) {
        is->audioClockTime = frame->pts + (double) frame->frame->nb_samples / frame->frame->sample_rate;
    } else {
        is->audioClockTime = NAN;
    }

    is->audioClockSerial = frame->serial;

    ALOGD(AUDIO_TAG, "%s audio: delay=%0.3f clock=%0.3f clock0=%0.3f", __func__, is->audioClockTime - is->lastAudioclockTime, is->audioClockTime, audio_clock0);
    is->lastAudioclockTime = is->audioClockTime;
    return resampled_data_size;
}

/* return the wanted number of samples to get better sync if sync_type is video
 * or external master clock */
int Audio::synchronizeAudio(int nbSamples) {

    VideoState *is = stream->getVideoState();
    int wantedNbSamples = nbSamples;

    /* if not master, then we try to remove or add samples to correct the clock */
    if (stream->getMasterSyncType() != SYNC_TYPE_AUDIO_MASTER) {
        double diff, avg_diff;
        int min_nb_samples, max_nb_samples;

        diff = is->audioClock.getClock() - stream->getMasterClock();

        if (!isnan(diff) && fabs(diff) < NO_SYNC_THRESHOLD) {
            is->audioDiffCum = diff + is->audioDiffAvgCoef * is->audioDiffCum;
            if (is->audioDiffAvgCount < AUDIO_DIFF_AVG_NB) {
                /* not enough measures to have a correct estimate */
                is->audioDiffAvgCount++;
            } else {
                /* estimate the A-V difference */
                avg_diff = is->audioDiffCum * (1.0 - is->audioDiffAvgCoef);

                if (fabs(avg_diff) >= is->audioDiffThreshold) {
                    wantedNbSamples = nbSamples + (int) (diff * is->audioSrc.freq);
                    min_nb_samples = ((nbSamples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    max_nb_samples = ((nbSamples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    wantedNbSamples = av_clip(wantedNbSamples, min_nb_samples, max_nb_samples);
                }
                ALOGD(AUDIO_TAG, "%s diff=%f adiff=%f sample_diff=%d apts=%0.3f %f", __func__, diff, avg_diff, wantedNbSamples - nbSamples, is->audioClockTime, is->audioDiffThreshold);
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
