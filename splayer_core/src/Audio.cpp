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
/// 从解码后的音频缓存队列中读取一帧，并做重采样处理(转码、变声、变速等操作)
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
        // 判断已解码的缓存队列是否可读
        if (!(frame = is->audioFrameQueue.peekReadable())) {
            ALOGD(AUDIO_TAG, "%s not readable ", __func__);
            return NEGATIVE(S_NOT_FRAME_READABLE);
        }
        // 缓存队列的下一帧
        is->audioFrameQueue.next();
    } while (frame->seekSerial != is->audioPacketQueue.seekSerial);

    // 解码的声道设计
    wantedChannelLayout = getWantedChannelLayout(frame);

    // 同步音频并获取采样的大小
    wantedNbSamples = synchronizeAudio(frame->frame->nb_samples);

    bool isNotSameSampleFormat = (AVSampleFormat) frame->frame->format != is->audioSrc.sampleFormat;
    bool isNotSameChannelLayout = wantedChannelLayout != is->audioSrc.channelLayout;
    bool isNotSameSampleRate = frame->frame->sample_rate != is->audioSrc.sampleRate;
    bool isNotSameNbSamples = wantedNbSamples != frame->frame->nb_samples && !is->audioSwrContext;
    bool isNeedConvert = isNotSameSampleFormat || isNotSameChannelLayout || isNotSameSampleRate || isNotSameNbSamples;

    // 如果跟源音频的格式、声道格式、采样率、采样大小等不相同，则需要做重采样处理
    if (isNeedConvert) {
        if (IS_NEGATIVE(initConvertSwrContext(is, wantedChannelLayout, frame))) {
            return NEGATIVE(S_NOT_INIT_CONVERSION_CONTEXT);
        }
    }

    // 如果重采样上下文存在，则进行重采样，否则直接复制当前的数据
    if (is->audioSwrContext) {
        int length = convertAudio(is, wantedNbSamples, frame);
        if (length > 0) {

            // 设置音频缓冲
            is->audioBuf = is->audioConvertBuf;

            // 计算重采样后的大小
            reSampledDataSize =
                    length * is->audioTarget.channels * av_get_bytes_per_sample(is->audioTarget.sampleFormat);

            // 这里可以做变声变速处理，请参考ijkplayer 的做法

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
    // 判断audioFrame 的pts 是否存在，如果存在，则更新音频时钟，否则置为无穷大(NAN)
    if (!isnan(frame->pts)) {
        is->audioClockTime = frame->pts + (double) frame->frame->nb_samples / frame->frame->sample_rate;
    } else {
        is->audioClockTime = NAN;
    }

    is->audioClockSeekSerial = frame->seekSerial;

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

    // 如果想要的采样大小跟帧的采样大小不一致，需要做补偿处理
    if (wantedNbSamples != frame->frame->nb_samples) {
        if (swr_set_compensation(is->audioSwrContext,
                                 (wantedNbSamples - frame->frame->nb_samples) * is->audioTarget.sampleRate /
                                 frame->frame->sample_rate,
                                 wantedNbSamples * is->audioTarget.sampleRate / frame->frame->sample_rate) < 0) {
            ALOGE(AUDIO_TAG, "%s swr_set_compensation() failed", __func__);
            return NEGATIVE(S_NOT_AUDIO_SWR_COMPENSATION);
        }
    }

    av_fast_malloc(&is->audioConvertBuf, &is->audioConvertBufSize, static_cast<size_t>(outSize));

    if (!is->audioConvertBuf) {
        return NEGATIVE(S_NOT_MEMORY);
    }

    // 音频重采样
    length = swr_convert(is->audioSwrContext, out, outCount, in, frame->frame->nb_samples);

    if (length < 0) {
        ALOGE(AUDIO_TAG, "%s swr_convert() failed", __func__);
        return NEGATIVE(S_NOT_CONVERT_AUDIO);
    }

    // 音频buffer缓冲太小了？
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

    // 重新设置音频的声道数、声道格式、采样频率、采样格式等

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
/// 同步音频
int Audio::synchronizeAudio(int nbSamples) {

    //TODO:

    VideoState *is = stream->getVideoState();
    int wantedNbSamples = nbSamples;

    // 如果不是以音频同步，则尝试通过移除或增加采样来纠正时钟
    /* if not master, then we try to remove or add samples to correct the clock */
    if (stream->getMasterSyncType() != SYNC_TYPE_AUDIO_MASTER) {
        double diff, avgDiff;
        int minNbSamples, maxNbSamples;

        // 获取音频时钟跟主时钟的差值
        diff = is->audioClock.getClock() - stream->getMasterClock();

        bool isNotNan = !isnan(diff);
        bool isNeedSync = fabs(diff) < NO_SYNC_THRESHOLD;

        // 判断差值是否存在，并且在非同步阈值范围内
        if (isNotNan && isNeedSync) {

            // 计算新的差值
            is->audioDiffCum = diff + is->audioDiffAvgCoef * is->audioDiffCum;

            // 记录差值的数量
            if (is->audioDiffAvgCount < AUDIO_DIFF_AVG_NB) {
                /* not enough measures to have a correct estimate */
                is->audioDiffAvgCount++;
            } else {

                // 估计音频和视频的时钟差值
                /* estimate the A-V difference */
                avgDiff = is->audioDiffCum * (1.0 - is->audioDiffAvgCoef);

                // 判断平均差值是否超过了音频差的阈值，如果超过，则计算新的采样值
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
            // 如果差值过大，重置防止pts出错
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

            // 取得解码帧
            audioSize = audioDecodeFrame();

            //  如果不存在音频帧，则输出静音
            if (audioSize < 0) {
                /* if error, just output silence */
                is->audioBuf = nullptr;
                is->audioBufSize = (unsigned int) (SDL_AUDIO_MIN_BUFFER_SIZE / is->audioTarget.frameSize *
                                                   is->audioTarget.frameSize);
            } else {
                // 显示音频波形
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

        // 如果不处于静音模式并且声音最大
        if (!is->audioMuted && is->audioBuf && is->audioVolume == MIX_MAX_VOLUME) {
            memcpy(stream, is->audioBuf + is->audioBufIndex, (size_t) (length));
        } else {
            memset(stream, 0, (size_t) length);
            // 非静音、并且音量不是最大，则需要混音
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
                     (double) (2 * is->audioHardwareBufSize + is->audioWriteBufSize) / is->audioTarget.bytesPerSec;
        int serial = is->audioClockSeekSerial;
        double time = audioCallbackTime / 1000000.0;
        is->audioClock.setClockAt(pts, serial, time);
        is->externalClock.syncClockToSlave(&is->audioClock);
    }
}

/// 更新采样输出
void Audio::update_sample_display(short *samples, int samples_size) {

}

void Audio::maxAudio(uint8_t *stream, uint8_t *buf, int index, int length, int volume) {

}

void Audio::updateVolume(int sign, double step) const {
    if (stream && stream->getVideoState()) {
        VideoState *is = stream->getVideoState();
        double volumeLevel = is->audioVolume ? (20 * log(is->audioVolume / (double) MIX_MAX_VOLUME) / log(10)) : -1000.0;
        int newVolume = lrint(MIX_MAX_VOLUME * pow(10.0, (volumeLevel + sign * step) / 20.0));
        is->audioVolume = av_clip(is->audioVolume == newVolume ? (is->audioVolume + sign) : newVolume, 0, MIX_MAX_VOLUME);
    }
}

