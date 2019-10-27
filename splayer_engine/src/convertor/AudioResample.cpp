
#include <convertor/AudioResample.h>

#include "convertor/AudioResample.h"

AudioResample::AudioResample() {
    srcFrame = av_frame_alloc();
}

AudioResample::~AudioResample() {
    av_frame_unref(srcFrame);
    av_free(srcFrame);
    srcFrame = nullptr;
    audioDecoder = nullptr;
    mediaSync = nullptr;
    playerState = nullptr;
}

int AudioResample::setReSampleParams(AudioDeviceSpec *obtainedSpec, int64_t wantedChannelLayout) {

    // 设置音频目标参数
    audioState->audioParamsTarget.sampleFormat = AV_SAMPLE_FMT_S16;
    audioState->audioParamsTarget.sampleRate = obtainedSpec->sampleRate;
    audioState->audioParamsTarget.channelLayout = wantedChannelLayout;
    audioState->audioParamsTarget.channels = obtainedSpec->channels;
    audioState->audioParamsTarget.frameSize = av_samples_get_buffer_size(nullptr,
                                                                         audioState->audioParamsTarget.channels,
                                                                         1,
                                                                         audioState->audioParamsTarget.sampleFormat,
                                                                         1);
    audioState->audioParamsTarget.bytesPerSec = av_samples_get_buffer_size(nullptr,
                                                                           audioState->audioParamsTarget.channels,
                                                                           audioState->audioParamsTarget.sampleRate,
                                                                           audioState->audioParamsTarget.sampleFormat,
                                                                           1);

    // 判断目标参数合法性
    if (audioState->audioParamsTarget.bytesPerSec <= 0 ||
        audioState->audioParamsTarget.frameSize <= 0) {
        ALOGE(TAG, "[%s] audio target params set fail", __func__);
        return ERROR;
    }

    audioState->audioParamsSrc = audioState->audioParamsTarget;
    audioState->audioHardwareBufSize = obtainedSpec->size;
    audioState->audioBufferSize = 0;
    audioState->audioBufferIndex = 0;
    audioState->audioDiffAvgCoef = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
    audioState->audioDiffAvgCount = 0;
    audioState->audioDiffThreshold =
            (double) (audioState->audioHardwareBufSize) / audioState->audioParamsTarget.bytesPerSec;

    if ((playerState->formatContext->iformat->flags &
         (AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK)) &&
        !playerState->formatContext->iformat->read_seek) {
        audioDecoder->setStartPts(
                playerState->formatContext->streams[playerState->audioIndex]->start_time);
        audioDecoder->setStartPtsTb(
                playerState->formatContext->streams[playerState->audioIndex]->time_base);
    }

    if (DEBUG) {
        ALOGD(TAG, "[%s] "
                   "sampleFormat = %s "
                   "sampleRate = %d "
                   "channelLayout = %lld "
                   "channels = %d "
                   "frameSize = %d "
                   "bytesPerSec = %d",
              __func__,
              av_get_sample_fmt_name((AVSampleFormat) audioState->audioParamsTarget.sampleFormat),
              audioState->audioParamsTarget.sampleRate,
              audioState->audioParamsTarget.channelLayout,
              audioState->audioParamsTarget.channels,
              audioState->audioParamsTarget.frameSize,
              audioState->audioParamsTarget.bytesPerSec
        );

        ALOGD(TAG, "[%s] "
                   "audioHardwareBufSize = %d "
                   "audioBufferSize = %d "
                   "audioBufferIndex = %d "
                   "audioDiffAvgCoef = %lf "
                   "audioDiffAvgCount = %d "
                   "audioDiffThreshold = %lf",
              __func__,
              audioState->audioHardwareBufSize,
              audioState->audioBufferSize,
              audioState->audioBufferIndex,
              audioState->audioDiffAvgCoef,
              audioState->audioDiffAvgCount,
              audioState->audioDiffThreshold
        );
    }

    return SUCCESS;
}

void AudioResample::onPCMDataCallback(uint8_t *stream, int len) {
    int bufferSize, length;

    // 没有音频解码器时，直接返回
    if (!audioDecoder) {
        memset(stream, 0, (size_t) (len));
        return;
    }

    audioState->audioCallbackTime = av_gettime_relative();

    while (len > 0) {
        if (audioState->audioBufferIndex >= audioState->audioBufferSize) {
            bufferSize = audioFrameReSample();
            if (bufferSize < 0) {
                audioState->audioOutputBuffer = nullptr;
                audioState->audioBufferSize = (unsigned int) (AUDIO_MIN_BUFFER_SIZE /
                                                              audioState->audioParamsTarget.frameSize *
                                                              audioState->audioParamsTarget.frameSize);
            } else {
                audioState->audioBufferSize = (unsigned int) bufferSize;
            }
            audioState->audioBufferIndex = 0;
        }

        length = audioState->audioBufferSize - audioState->audioBufferIndex;

        if (length > len) {
            length = len;
        }

        // 复制经过转码输出的PCM数据到缓冲区中
        if (audioState->audioOutputBuffer && !playerState->audioMute) {
            memcpy(stream, audioState->audioOutputBuffer + audioState->audioBufferIndex,
                   (size_t) length);
        } else {
            memset(stream, 0, (size_t) length);
        }

        len -= length;
        stream += length;

        audioState->audioBufferIndex += length;
    }

    audioState->audioWriteBufferSize = audioState->audioBufferSize - audioState->audioBufferIndex;

    if (!isnan(audioState->audioClock) && mediaSync) {
        double pts = audioState->audioClock - (double) (2 * audioState->audioHardwareBufSize +
                                                        audioState->audioWriteBufferSize) /
                                              audioState->audioParamsTarget.bytesPerSec;
        double time = audioState->audioCallbackTime / 1000000.0;
        mediaSync->updateAudioClock(pts, audioState->seekSerial, time);
    }
}

int AudioResample::audioSynchronize(int nbSamples) {
    int wantedNbSamples = nbSamples;

    // 如果时钟不是同步到音频流，则需要进行对音频频进行同步处理
    if (playerState->syncType != AV_SYNC_AUDIO) {

        double diff, avg_diff;

        int minNbSamples, maxNbSamples;

        diff = mediaSync ? mediaSync->getAudioDiffClock() : 0;

        if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD) {

            audioState->audioDiffCum =
                    diff + audioState->audioDiffAvgCoef * audioState->audioDiffCum;

            if (audioState->audioDiffAvgCount < AUDIO_DIFF_AVG_NB) {
                audioState->audioDiffAvgCount++;
            } else {
                avg_diff = audioState->audioDiffCum * (1.0 - audioState->audioDiffAvgCoef);

                if (fabs(avg_diff) >= audioState->audioDiffThreshold) {
                    wantedNbSamples =
                            nbSamples + (int) (diff * audioState->audioParamsSrc.sampleRate);

                    minNbSamples = ((nbSamples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    maxNbSamples = ((nbSamples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100));

                    wantedNbSamples = av_clip(wantedNbSamples, minNbSamples, maxNbSamples);
                }
            }
        } else {
            audioState->audioDiffAvgCount = 0;
            audioState->audioDiffCum = 0;
        }
    }

    return wantedNbSamples;
}

int AudioResample::audioFrameReSample() {
    int reSampledDataSize = 0;
    int64_t wantedChannelLayout = 0;
    int wantedNbSamples = 0;
    Frame *frame = nullptr;

    // 处于暂停状态
    if (!audioDecoder || !audioDecoder->getFrameQueue() || playerState->pauseRequest) {
        return ERROR;
    }

    do {
        // 判断已解码的缓存队列是否可读
        if (!(frame = audioDecoder->getFrameQueue()->peekReadable())) {
            if (DEBUG) {
                ALOGD(TAG, "[%s] audio peek readable ", __func__);
            }
            return ERROR_AUDIO_PEEK_READABLE;
        }
        av_frame_move_ref(srcFrame, frame->frame);
        // 缓存队列的下一帧
        audioDecoder->getFrameQueue()->popFrame();
    } while (frame->seekSerial != audioDecoder->getPacketQueue()->getLastSeekSerial());

    // 解码的声道设计
    wantedChannelLayout = srcFrame->channel_layout && (srcFrame->channels ==
                                                       av_get_channel_layout_nb_channels(
                                                               srcFrame->channel_layout))
                          ? srcFrame->channel_layout : av_get_default_channel_layout(
                    srcFrame->channels);

    // 同步音频并获取采样的大小
    wantedNbSamples = audioSynchronize(srcFrame->nb_samples);

    bool isNotSameSampleFormat =
            (AVSampleFormat) srcFrame->format != audioState->audioParamsSrc.sampleFormat;
    bool isNotSameChannelLayout = wantedChannelLayout != audioState->audioParamsSrc.channelLayout;
    bool isNotSameSampleRate = srcFrame->sample_rate != audioState->audioParamsSrc.sampleRate;
    bool isNotSameNbSamples = wantedNbSamples != srcFrame->nb_samples && !audioState->swrContext;
    bool isNeedConvert = isNotSameSampleFormat || isNotSameChannelLayout || isNotSameSampleRate ||
                         isNotSameNbSamples;

    if (DEBUG) {
        ALOGD(TAG, "[%s] "
                   "isNotSameSampleFormat = %d "
                   "isNotSameChannelLayout = %d "
                   "isNotSameSampleRate = %d "
                   "isNotSameNbSamples = %d "
                   "isNeedConvert = %d ",
              __func__,
              isNotSameSampleFormat,
              isNotSameChannelLayout,
              isNotSameSampleRate,
              isNotSameNbSamples,
              isNeedConvert
        );
    }

    // 如果跟源音频的格式、声道格式、采样率、采样大小等不相同，则需要做重采样处理
    if (isNeedConvert && initConvertSwrContext(wantedChannelLayout, srcFrame) < 0) {
        return ERROR_AUDIO_SWR;
    }

    // 音频重采样处理
    if (audioState->swrContext) {
        int length = convertAudio(wantedNbSamples, srcFrame);

        if (length > 0) {
            audioState->audioOutputBuffer = audioState->reSampleBuffer;
            reSampledDataSize = length * audioState->audioParamsTarget.channels *
                                av_get_bytes_per_sample(audioState->audioParamsTarget.sampleFormat);

            // 这里可以做变声变速处理，请参考ijkplayer 的做法

// 变速变调处理
//            if ((playerInfoStatus->playbackRate != 1.0f ||
//                 playerInfoStatus->playbackPitch != 1.0f) &&
//                !playerInfoStatus->abortRequest) {
//                int bytes_per_sample =
//                        av_get_bytes_per_sample(audioState->audioParamsTarget.sampleFormat);
//                av_fast_malloc(&audioState->soundTouchBuffer,
//                               &audioState->soundTouchBufferSize,
//                               (size_t) out_size * translate_time);
//                for (int i = 0; i < (reSampledDataSize / 2); i++) {
//                    audioState->soundTouchBuffer[i] =
//                            (audioState->reSampleBuffer[i * 2] |
//                             (audioState->reSampleBuffer[i * 2 + 1] << 8));
//                }
//                if (!soundTouchWrapper) {
//                    soundTouchWrapper = new SoundTouchWrapper();
//                }
//                int ret_len = soundTouchWrapper->translate(
//                        audioState->soundTouchBuffer, (float) (playerInfoStatus->playbackRate),
//                        (float) (playerInfoStatus->playbackPitch != 1.0f
//                                 ? playerInfoStatus->playbackPitch
//                                 : 1.0f / playerInfoStatus->playbackRate),
//                        reSampledDataSize / 2, bytes_per_sample,
//                        audioState->audioParamsTarget.channels, frame->frame->sample_rate);
//                if (ret_len > 0) {
//                    audioState->audioOutputBuffer = (uint8_t *) audioState->soundTouchBuffer;
//                    reSampledDataSize = ret_len;
//                } else {
//                    translate_time++;
//                    av_frame_unref(frame->frame);
//                    continue;
//                }
//            }
        } else {
            return ERROR_AUDIO_SWR_CONVERT;
        }

    } else {
        audioState->audioOutputBuffer = srcFrame->data[0];
        reSampledDataSize = av_samples_get_buffer_size(nullptr, srcFrame->channels,
                                                       srcFrame->nb_samples,
                                                       (AVSampleFormat) srcFrame->format, 1);;
    }

    // 利用pts更新音频时钟
    if (srcFrame->pts != AV_NOPTS_VALUE) {
        audioState->audioClock = srcFrame->pts * av_q2d((AVRational) {1, srcFrame->sample_rate}) +
                                 (double) srcFrame->nb_samples / srcFrame->sample_rate;
    } else {
        audioState->audioClock = NAN;
    }

    audioState->seekSerial = frame->seekSerial;

    // 使用完成释放引用，防止内存泄漏
    av_frame_unref(srcFrame);

    return reSampledDataSize;
}

int AudioResample::initConvertSwrContext(int64_t desireChannelLayout, AVFrame *frame) const {
    swr_free(&audioState->swrContext);
    audioState->swrContext = swr_alloc_set_opts(
            nullptr,
            audioState->audioParamsTarget.channelLayout,
            audioState->audioParamsTarget.sampleFormat,
            audioState->audioParamsTarget.sampleRate,
            desireChannelLayout,
            (AVSampleFormat) frame->format,
            frame->sample_rate,
            0,
            nullptr);

    if (!audioState->swrContext || swr_init(audioState->swrContext) < 0) {
        ALOGE(TAG,
              "[%s] Cannot init sample rate converter for conversion of %d Hz "
              "%s %d channels to %d Hz %s %d channels!",
              __func__,
              frame->sample_rate,
              av_get_sample_fmt_name((AVSampleFormat) frame->format),
              frame->channels, audioState->audioParamsTarget.sampleRate,
              av_get_sample_fmt_name(audioState->audioParamsTarget.sampleFormat),
              audioState->audioParamsTarget.channels);
        swr_free(&audioState->swrContext);
        return ERROR_AUDIO_SWR;
    }

    audioState->audioParamsSrc.channelLayout = desireChannelLayout;
    audioState->audioParamsSrc.channels = frame->channels;
    audioState->audioParamsSrc.sampleRate = frame->sample_rate;
    audioState->audioParamsSrc.sampleFormat = (AVSampleFormat) frame->format;

    if (DEBUG) {
        ALOGD(TAG, "[%s] "
                   "sampleFormat = %s "
                   "sampleRate = %d "
                   "channelLayout = %lld "
                   "channels = %d "
                   "frameSize = %d "
                   "bytesPerSec = %d",
              __func__,
              av_get_sample_fmt_name((AVSampleFormat) audioState->audioParamsSrc.sampleFormat),
              audioState->audioParamsSrc.sampleRate,
              audioState->audioParamsSrc.channelLayout,
              audioState->audioParamsSrc.channels,
              audioState->audioParamsSrc.frameSize,
              audioState->audioParamsSrc.bytesPerSec
        );
    }
    return SUCCESS;
}

int AudioResample::convertAudio(int wantedNbSamples, AVFrame *frame) const {

    const auto **in = (const uint8_t **) frame->extended_data;

    uint8_t **out = &audioState->reSampleBuffer;

    int outCount = static_cast<int>(
            (int64_t) wantedNbSamples * audioState->audioParamsTarget.sampleRate /
            frame->sample_rate + 256);

    int outSize = av_samples_get_buffer_size(nullptr, audioState->audioParamsTarget.channels,
                                             outCount, audioState->audioParamsTarget.sampleFormat,
                                             0);

    int length;

    if (outSize < 0) {
        ALOGE(TAG, "av_samples_get_buffer_size() failed");
        return ERROR_AUDIO_OUT_SIZE;
    }

    if (wantedNbSamples != frame->nb_samples) {
        if (swr_set_compensation(audioState->swrContext, (wantedNbSamples - frame->nb_samples) *
                                                         audioState->audioParamsTarget.sampleRate /
                                                         frame->sample_rate,
                                 wantedNbSamples * audioState->audioParamsTarget.sampleRate /
                                 frame->sample_rate) < 0) {
            ALOGE(TAG, "swr_set_compensation() failed");
            return ERROR_AUDIO_SWR_COMPENSATION;
        }
    }

    av_fast_malloc(&audioState->reSampleBuffer, &audioState->reSampleSize, (size_t) outSize);

    if (!audioState->reSampleBuffer) {
        return ERROR_NOT_MEMORY;
    }

    length = swr_convert(audioState->swrContext, out, outCount, in, frame->nb_samples);

    if (length < 0) {
        ALOGE(TAG, "[%s] swr_convert() failed", __func__);
        return ERROR;
    }

    // 音频buffer缓冲太小了？
    if (length == outCount) {
        if (DEBUG) {
            ALOGD(TAG, "[%s] audio buffer is probably too small", __func__);
        }
        if (swr_init(audioState->swrContext) < 0) {
            swr_free(&audioState->swrContext);
        }
    }
    return length;
}

int AudioResample::create() {
    audioState = (AudioState *) av_mallocz(sizeof(AudioState));
    memset(audioState, 0, sizeof(AudioState));
    soundTouchWrapper = new SoundTouchWrapper();
    return SUCCESS;
}

int AudioResample::destroy() {
    if (soundTouchWrapper) {
        delete soundTouchWrapper;
        soundTouchWrapper = nullptr;
    }
    if (audioState) {
        swr_free(&audioState->swrContext);
        av_freep(&audioState->reSampleBuffer);
        memset(audioState, 0, sizeof(AudioState));
        av_free(audioState);
        audioState = nullptr;
    }
    return SUCCESS;
}

void AudioResample::setPlayerState(PlayerInfoStatus *playerState) {
    AudioResample::playerState = playerState;
}

void AudioResample::setMediaSync(MediaSync *mediaSync) {
    AudioResample::mediaSync = mediaSync;
}

void AudioResample::setAudioDecoder(AudioDecoder *audioDecoder) {
    AudioResample::audioDecoder = audioDecoder;
}
