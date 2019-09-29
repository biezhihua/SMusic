
#include <convertor/AudioReSampler.h>

#include "convertor/AudioReSampler.h"

AudioReSampler::AudioReSampler() {}

AudioReSampler::~AudioReSampler() {
    audioDecoder = nullptr;
    mediaSync = nullptr;
    playerState = nullptr;
}

int AudioReSampler::setReSampleParams(AudioDeviceSpec *spec,
                                      int64_t wanted_channel_layout) {
    audioState->audioParamsTarget.sampleFormat = AV_SAMPLE_FMT_S16;
    audioState->audioParamsTarget.sampleRate = spec->freq;
    audioState->audioParamsTarget.channelLayout = wanted_channel_layout;
    audioState->audioParamsTarget.channels = spec->channels;
    audioState->audioParamsTarget.frameSize = av_samples_get_buffer_size(
            nullptr, audioState->audioParamsTarget.channels, 1,
            audioState->audioParamsTarget.sampleFormat, 1);
    audioState->audioParamsTarget.bytesPerSec = av_samples_get_buffer_size(
            nullptr, audioState->audioParamsTarget.channels,
            audioState->audioParamsTarget.sampleRate,
            audioState->audioParamsTarget.sampleFormat, 1);

    if (audioState->audioParamsTarget.bytesPerSec <= 0 ||
        audioState->audioParamsTarget.frameSize <= 0) {
        if (DEBUG) {
            ALOGE(TAG, "av_samples_get_buffer_size failed");
        }
        return ERROR;
    }

    audioState->audioParamsSrc = audioState->audioParamsTarget;
    audioState->audioHardwareBufSize = spec->size;
    audioState->bufferSize = 0;
    audioState->bufferIndex = 0;
    audioState->audio_diff_avg_coef = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
    audioState->audio_diff_avg_count = 0;
    audioState->audio_diff_threshold =
            (double) (audioState->audioHardwareBufSize) /
            audioState->audioParamsTarget.bytesPerSec;

    if ((playerState->formatContext->iformat->flags &
         (AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK)) &&
        !playerState->formatContext->iformat->read_seek) {
        audioDecoder->setStartPts(
                playerState->formatContext->streams[playerState->audioIndex]
                        ->start_time);
        audioDecoder->setStartPtsTb(
                playerState->formatContext->streams[playerState->audioIndex]
                        ->time_base);
    }

    return SUCCESS;
}

void AudioReSampler::pcmQueueCallback(uint8_t *stream, int len) {
    int bufferSize, length;

    // 没有音频解码器时，直接返回
    if (audioDecoder == nullptr) {
        memset(stream, 0, (size_t) (len));
        return;
    }

    audioState->audio_callback_time = av_gettime_relative();
    while (len > 0) {
        if (audioState->bufferIndex >= audioState->bufferSize) {
            bufferSize = audioFrameReSample();
            if (bufferSize < 0) {
                audioState->outputBuffer = nullptr;
                audioState->bufferSize =
                        (unsigned int) (AUDIO_MIN_BUFFER_SIZE /
                                        audioState->audioParamsTarget.frameSize *
                                        audioState->audioParamsTarget.frameSize);
            } else {
                audioState->bufferSize = (unsigned int) bufferSize;
            }
            audioState->bufferIndex = 0;
        }

        length = audioState->bufferSize - audioState->bufferIndex;
        if (length > len) {
            length = len;
        }
        // 复制经过转码输出的PCM数据到缓冲区中
        if (audioState->outputBuffer != nullptr && !playerState->audioMute) {
            memcpy(stream, audioState->outputBuffer + audioState->bufferIndex,
                   (size_t) length);
        } else {
            memset(stream, 0, (size_t) length);
        }
        len -= length;
        stream += length;
        audioState->bufferIndex += length;
    }
    audioState->writeBufferSize =
            audioState->bufferSize - audioState->bufferIndex;

    if (!isnan(audioState->audioClock) && mediaSync) {
        double pts =
                audioState->audioClock - (double) (2 * audioState->audioHardwareBufSize +
                                                   audioState->writeBufferSize) /
                                         audioState->audioParamsTarget.bytesPerSec;
        double time = audioState->audio_callback_time / 1000000.0;
        mediaSync->updateAudioClock(pts, audioState->seekSerial, time);
    }
}

int AudioReSampler::audioSynchronize(int nbSamples) {
    int wanted_nb_samples = nbSamples;

    // 如果时钟不是同步到音频流，则需要进行对音频频进行同步处理
    if (playerState->syncType != AV_SYNC_AUDIO) {
        double diff, avg_diff;
        int min_nb_samples, max_nb_samples;
        diff = mediaSync ? mediaSync->getAudioDiffClock() : 0;
        if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD) {
            audioState->audio_diff_cum =
                    diff + audioState->audio_diff_avg_coef * audioState->audio_diff_cum;
            if (audioState->audio_diff_avg_count < AUDIO_DIFF_AVG_NB) {
                audioState->audio_diff_avg_count++;
            } else {
                avg_diff = audioState->audio_diff_cum *
                           (1.0 - audioState->audio_diff_avg_coef);

                if (fabs(avg_diff) >= audioState->audio_diff_threshold) {
                    wanted_nb_samples =
                            nbSamples + (int) (diff * audioState->audioParamsSrc.sampleRate);
                    min_nb_samples =
                            ((nbSamples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    max_nb_samples =
                            ((nbSamples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    wanted_nb_samples =
                            av_clip(wanted_nb_samples, min_nb_samples, max_nb_samples);
                }
            }
        } else {
            audioState->audio_diff_avg_count = 0;
            audioState->audio_diff_cum = 0;
        }
    }

    return wanted_nb_samples;
}

int AudioReSampler::audioFrameReSample() {
    int reSampledDataSize;
    int64_t wantedChannelLayout;
    int wantedNbSamples;
    Frame *frame = nullptr;

    // 处于暂停状态
    if (!audioDecoder || playerState->abortRequest || playerState->pauseRequest) {
        return -1;
    }

    do {
        // 判断已解码的缓存队列是否可读
        if (!(frame = audioDecoder->getFrameQueue()->peekReadable())) {
            if (DEBUG) {
                ALOGE(TAG, "%s audio peek readable ", __func__);
            }
            return ERROR_AUDIO_PEEK_READABLE;
        }
        // 缓存队列的下一帧
        audioDecoder->getFrameQueue()->popFrame();
    } while (frame->seekSerial != audioDecoder->getPacketQueue()->getLastSeekSerial());

    // 解码的声道设计
    wantedChannelLayout = getWantedChannelLayout(frame);

    // 同步音频并获取采样的大小
    wantedNbSamples = audioSynchronize(frame->frame->nb_samples);

    bool isNotSameSampleFormat = (AVSampleFormat) frame->frame->format != audioState->audioParamsSrc.sampleFormat;
    bool isNotSameChannelLayout = wantedChannelLayout != audioState->audioParamsSrc.channelLayout;
    bool isNotSameSampleRate = frame->frame->sample_rate != audioState->audioParamsSrc.sampleRate;
    bool isNotSameNbSamples = wantedNbSamples != frame->frame->nb_samples && !audioState->swrContext;
    bool isNeedConvert =
            isNotSameSampleFormat || isNotSameChannelLayout || isNotSameSampleRate || isNotSameNbSamples;

    // 如果跟源音频的格式、声道格式、采样率、采样大小等不相同，则需要做重采样处理
    if (isNeedConvert && initConvertSwrContext(wantedChannelLayout, frame) < 0) {
        return ERROR_AUDIO_SWR;
    }

// 音频重采样处理
    if (audioState->swrContext) {
        int length = convertAudio(wantedNbSamples, frame);

        if (length > 0) {
            audioState->
                    outputBuffer = audioState->reSampleBuffer;
            reSampledDataSize =
                    length * audioState->audioParamsTarget.channels *
                    av_get_bytes_per_sample(audioState->audioParamsTarget.sampleFormat);

// 这里可以做变声变速处理，请参考ijkplayer 的做法
        } else {
            return ERROR_AUDIO_SWR_CONVERT;
        }

// 变速变调处理
//            if ((playerState->playbackRate != 1.0f ||
//                 playerState->playbackPitch != 1.0f) &&
//                !playerState->abortRequest) {
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
//                        audioState->soundTouchBuffer, (float) (playerState->playbackRate),
//                        (float) (playerState->playbackPitch != 1.0f
//                                 ? playerState->playbackPitch
//                                 : 1.0f / playerState->playbackRate),
//                        reSampledDataSize / 2, bytes_per_sample,
//                        audioState->audioParamsTarget.channels, frame->frame->sample_rate);
//                if (ret_len > 0) {
//                    audioState->outputBuffer = (uint8_t *) audioState->soundTouchBuffer;
//                    reSampledDataSize = ret_len;
//                } else {
//                    translate_time++;
//                    av_frame_unref(frame->frame);
//                    continue;
//                }
//            }
    } else {
        audioState->
                outputBuffer = frame->frame->data[0];
        reSampledDataSize = av_samples_get_buffer_size(nullptr,
                                                       frame->frame->channels,
                                                       frame->frame->nb_samples,
                                                       (AVSampleFormat) frame->format, 1);;
    }

// 利用pts更新音频时钟
    if (frame->pts != AV_NOPTS_VALUE) {
        audioState->
                audioClock =
                frame->pts * av_q2d((AVRational) {1, frame->frame->sample_rate}) +
                (double) frame->frame->nb_samples / frame->frame->sample_rate;
    } else {
        audioState->
                audioClock = NAN;
    }

    audioState->
            seekSerial = frame->seekSerial;

// 使用完成释放引用，防止内存泄漏
    av_frame_unref(frame
                           ->frame);

    return reSampledDataSize;
}

int AudioReSampler::initConvertSwrContext(int64_t wantedChannelLayout, Frame *frame) const {
    swr_free(&audioState->swrContext);
    audioState->swrContext = swr_alloc_set_opts(
            nullptr, audioState->audioParamsTarget.channelLayout,
            audioState->audioParamsTarget.sampleFormat,
            audioState->audioParamsTarget.sampleRate, wantedChannelLayout,
            (AVSampleFormat) frame->frame->format, frame->frame->sample_rate, 0, nullptr);

    if (!audioState->swrContext || swr_init(audioState->swrContext) < 0) {
        if (DEBUG)
            ALOGE(TAG,
                  "Cannot create sample rate converter for conversion of %d Hz "
                  "%s %d channels to %d Hz %s %d channels!",
                  frame->frame->sample_rate,
                  av_get_sample_fmt_name((AVSampleFormat) frame->format),
                  frame->frame->channels, audioState->audioParamsTarget.sampleRate,
                  av_get_sample_fmt_name(
                          audioState->audioParamsTarget.sampleFormat),
                  audioState->audioParamsTarget.channels);
        swr_free(&audioState->swrContext);
        return ERROR_AUDIO_SWR;
    }
    audioState->audioParamsSrc.channelLayout = wantedChannelLayout;
    audioState->audioParamsSrc.channels = frame->frame->channels;
    audioState->audioParamsSrc.sampleRate = frame->frame->sample_rate;
    audioState->audioParamsSrc.sampleFormat = (AVSampleFormat) frame->format;
    return SUCCESS;
}

int AudioReSampler::convertAudio(int wantedNbSamples, Frame *frame) const {
    const uint8_t **in = (const uint8_t **) frame->frame->extended_data;
    uint8_t **out = &audioState->reSampleBuffer;

    int outCount = static_cast<int>(
            (int64_t) wantedNbSamples * audioState->audioParamsTarget.sampleRate / frame->frame->sample_rate +
            256);

    int outSize = av_samples_get_buffer_size(nullptr,
                                             audioState->audioParamsTarget.channels,
                                             outCount,
                                             audioState->audioParamsTarget.sampleFormat,
                                             0);

    int length;

    if (outSize < 0) {
        if (DEBUG) {
            ALOGE(TAG, "av_samples_get_buffer_size() failed");
        }
        return ERROR_AUDIO_OUT_SIZE;
    }

    if (wantedNbSamples != frame->frame->nb_samples) {
        if (swr_set_compensation(audioState->swrContext,
                                 (wantedNbSamples - frame->frame->nb_samples) *
                                 audioState->audioParamsTarget.sampleRate /
                                 frame->frame->sample_rate,
                                 wantedNbSamples *
                                 audioState->audioParamsTarget.sampleRate /
                                 frame->frame->sample_rate) < 0) {
            if (DEBUG) ALOGE(TAG, "swr_set_compensation() failed");
            return ERROR_AUDIO_SWR_COMPENSATION;
        }
    }
    av_fast_malloc(&audioState->reSampleBuffer, &audioState->reSampleSize, outSize);
    if (!audioState->reSampleBuffer) {
        return ERROR_NOT_MEMORY;
    }
    length = swr_convert(audioState->swrContext, out, outCount, in,
                         frame->frame->nb_samples);

    if (length < 0) {
        if (DEBUG) ALOGE(TAG, "%s swr_convert() failed", __func__);
        return ERROR;
    }

    // 音频buffer缓冲太小了？
    if (length == outCount) {
        if (DEBUG) ALOGE(TAG, "%s audio buffer is probably too small", __func__);
        if (swr_init(audioState->swrContext) < 0) {
            swr_free(&audioState->swrContext);
        }
    }
    return length;
}

uint64_t AudioReSampler::getWantedChannelLayout(Frame *frame) const {
    bool isValid = frame->frame->channel_layout &&
                   frame->frame->channels == av_get_channel_layout_nb_channels(frame->frame->channel_layout);
    return isValid ? frame->frame->channel_layout : av_get_default_channel_layout(frame->frame->channels);
}

void AudioReSampler::create() {
    audioState = (AudioState *) av_mallocz(sizeof(AudioState));
    memset(audioState, 0, sizeof(AudioState));
    soundTouchWrapper = new SoundTouchWrapper();
}

void AudioReSampler::destroy() {
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
}

void AudioReSampler::setPlayerState(PlayerState *playerState) {
    AudioReSampler::playerState = playerState;
}

void AudioReSampler::setMediaSync(MediaSync *mediaSync) {
    AudioReSampler::mediaSync = mediaSync;
}

void AudioReSampler::setAudioDecoder(AudioDecoder *audioDecoder) {
    AudioReSampler::audioDecoder = audioDecoder;
}
