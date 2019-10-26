#include "AndroidAudioDevice.h"

AndroidAudioDevice::AndroidAudioDevice() : AudioDevice() {

}

AndroidAudioDevice::~AndroidAudioDevice() = default;

/**
 * SLES缓冲回调
 */
void slBufferPCMCallBack(SLAndroidSimpleBufferQueueItf bf, void *context) {

}

/// 打开音频设备，并返回缓冲区大小
int AndroidAudioDevice::open(AudioDeviceSpec *desired, AudioDeviceSpec *obtained) {

    SLresult result;

    // create engine
    result = slCreateEngine(&slObject, 0, nullptr, 0, nullptr, nullptr);
    if ((result) != SL_RESULT_SUCCESS) {
        ALOGE(TAG, "%s: slCreateEngine() failed", __func__);
        return -1;
    }

    // realize the engine
    result = (*slObject)->Realize(slObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        ALOGE(TAG, "%s: slObject->Realize() failed", __func__);
        return -1;
    }

    // get the engine interface, which is needed in order to create other objects
    result = (*slObject)->GetInterface(slObject, SL_IID_ENGINE, &slEngine);
    if (result != SL_RESULT_SUCCESS) {
        ALOGE(TAG, "%s: slObject->GetInterface() failed", __func__);
        return -1;
    }

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*slEngine)->CreateOutputMix(slEngine, &slOutputMixObject, 1, mids, mreq);
    if (result != SL_RESULT_SUCCESS) {
        ALOGE(TAG, "%s: slEngine->CreateOutputMix() failed", __func__);
        return -1;
    }

    // realize the output mix
    result = (*slOutputMixObject)->Realize(slOutputMixObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        ALOGE(TAG, "%s: slOutputMixObject->Realize() failed", __func__);
        return -1;
    }

    // configure audio sink
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, slOutputMixObject};
    SLDataSink audioSink = {&outputMix, nullptr};

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
            OPENSLES_BUFFERS
    };
    SLuint32 channelMask;
    switch (desired->channels) {
        case 2: {
            channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
            break;
        }
        case 1: {
            channelMask = SL_SPEAKER_FRONT_CENTER;
            break;
        }
        default: {
            ALOGE(TAG, "%s, invalid channel %d", __func__, desired->channels);
            return -1;
        }
    }
    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM,              // 播放器PCM格式
            desired->channels,              // 声道数
            getSLSampleRate(desired->sampleRate), // SL采样率
            SL_PCMSAMPLEFORMAT_FIXED_16,    // 位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,    // 和位数一致
            channelMask,                    // 格式
            SL_BYTEORDER_LITTLEENDIAN       // 小端存储
    };

    //
    SLDataSource slDataSource = {&android_queue, &format_pcm};

    /*
     * create audio player:
     *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
     *     for fast audio case
     */
    const SLInterfaceID ids[3] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_VOLUME, SL_IID_PLAY};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*slEngine)->CreateAudioPlayer(slEngine, &slPlayerObject, &slDataSource, &audioSink, 3,
                                            ids, req);
    if (result != SL_RESULT_SUCCESS) {
        ALOGE(TAG, "%s: slEngine->CreateAudioPlayer() failed", __func__);
        return -1;
    }

    // realize the player
    result = (*slPlayerObject)->Realize(slPlayerObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        ALOGE(TAG, "%s: slPlayerObject->Realize() failed", __func__);
        return -1;
    }

    // get the play interface
    result = (*slPlayerObject)->GetInterface(slPlayerObject, SL_IID_PLAY, &slPlayItf);
    if (result != SL_RESULT_SUCCESS) {
        ALOGE(TAG, "%s: slPlayerObject->GetInterface(SL_IID_PLAY) failed", __func__);
        return -1;
    }

    // get the volume interface
    result = (*slPlayerObject)->GetInterface(slPlayerObject, SL_IID_VOLUME, &slVolumeItf);
    if (result != SL_RESULT_SUCCESS) {
        ALOGE(TAG, "%s: slPlayerObject->GetInterface(SL_IID_VOLUME) failed", __func__);
        return -1;
    }

    // get the buffer queue interface
    result = (*slPlayerObject)->GetInterface(slPlayerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                             &slBufferQueueItf);
    if (result != SL_RESULT_SUCCESS) {
        ALOGE(TAG, "%s: slPlayerObject->GetInterface(SL_IID_ANDROIDSIMPLEBUFFERQUEUE) failed",
              __func__);
        return -1;
    }

    // register callback on the buffer queue
    result = (*slBufferQueueItf)->RegisterCallback(slBufferQueueItf, slBufferPCMCallBack, this);
    if (result != SL_RESULT_SUCCESS) {
        ALOGE(TAG, "%s: slBufferQueueItf->RegisterCallback() failed", __func__);
        return -1;
    }

    // 这里计算缓冲区大小等参数

    // 一帧占多少字节
    bytes_per_frame = format_pcm.numChannels * format_pcm.bitsPerSample / 8;
    // 每个缓冲区占多少毫秒
    milli_per_buffer = OPENSLES_BUFLEN;
    // 一个缓冲区有多少帧数据
    frames_per_buffer = milli_per_buffer * format_pcm.samplesPerSec / 1000000;
    // 一个缓冲区大小
    bytes_per_buffer = bytes_per_frame * frames_per_buffer;
    // 缓冲区总大小
    buffer_capacity = static_cast<size_t>(OPENSLES_BUFFERS * bytes_per_buffer);

    ALOGD(TAG, "OpenSL-ES: bytes_per_frame  = %d bytes", bytes_per_frame);
    ALOGD(TAG, "OpenSL-ES: milli_per_buffer = %d ms", milli_per_buffer);
    ALOGD(TAG, "OpenSL-ES: frame_per_buffer = %d frames", frames_per_buffer);
    ALOGD(TAG, "OpenSL-ES: buffer_capacity  = %d bytes", buffer_capacity);
    ALOGD(TAG, "OpenSL-ES: buffer_capacity  = %d bytes", (int) buffer_capacity);

    if (obtained != nullptr) {
        *obtained = *desired;
        obtained->size = (uint32_t) buffer_capacity;
        obtained->sampleRate = format_pcm.samplesPerSec / 1000;
    }
    audioDeviceSpec = *desired;

    // 创建缓冲区
    buffer = (uint8_t *) malloc(buffer_capacity);
    if (!buffer) {
        ALOGE(TAG, "%s: failed to alloc buffer %d", __func__, (int) buffer_capacity);
        return -1;
    }

    // 填充缓冲区数据
    memset(buffer, 0, buffer_capacity);
    for (int i = 0; i < OPENSLES_BUFFERS; ++i) {
        result = (*slBufferQueueItf)->Enqueue(slBufferQueueItf,
                                              buffer + i * bytes_per_buffer,
                                              bytes_per_buffer);
        if (result != SL_RESULT_SUCCESS) {
            ALOGE(TAG, "%s: slBufferQueueItf->Enqueue(000...) failed", __func__);
        }
    }

    if (DEBUG) {
        ALOGD(TAG, "open SLES Device success");
    }

    // 返回缓冲大小
    return buffer_capacity;
}

int AndroidAudioDevice::create() {
    slObject = nullptr;
    slEngine = nullptr;
    slOutputMixObject = nullptr;
    slPlayerObject = nullptr;
    slPlayItf = nullptr;
    slVolumeItf = nullptr;
    slBufferQueueItf = nullptr;
    memset(&audioDeviceSpec, 0, sizeof(AudioDeviceSpec));
    abortRequest = 1;
    pauseRequest = 0;
    flushRequest = 0;
    audioThread = nullptr;
    updateVolume = false;
    return SUCCESS;
}

void AndroidAudioDevice::destroy() {
    mutex.lock();
    memset(&audioDeviceSpec, 0, sizeof(AudioDeviceSpec));
    if (slPlayerObject != nullptr) {
        (*slPlayerObject)->Destroy(slPlayerObject);
        slPlayerObject = nullptr;
        slPlayItf = nullptr;
        slVolumeItf = nullptr;
        slBufferQueueItf = nullptr;
    }

    if (slOutputMixObject != nullptr) {
        (*slOutputMixObject)->Destroy(slOutputMixObject);
        slOutputMixObject = nullptr;
    }

    if (slObject != nullptr) {
        (*slObject)->Destroy(slObject);
        slObject = nullptr;
        slEngine = nullptr;
    }
    mutex.unlock();
}

void AndroidAudioDevice::start() {
    // 回调存在时，表示成功打开SLES音频设备，另外开一个线程播放音频
    if (audioDeviceSpec.callback != nullptr) {
        abortRequest = 0;
        pauseRequest = 0;
        if (!audioThread) {
            audioThread = new Thread(this, Priority_High);
            audioThread->start();
        }
    } else {
        if (DEBUG) {
            ALOGE(TAG, "%s audio device callback is NULL!", __func__);
        }
    }
}

void AndroidAudioDevice::stop() {
    if (DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }

    mutex.lock();
    abortRequest = 1;
    condition.signal();
    mutex.unlock();

    if (audioThread) {
        audioThread->join();
        delete audioThread;
        audioThread = nullptr;
    }
}

void AndroidAudioDevice::pause() {
    mutex.lock();
    pauseRequest = 1;
    condition.signal();
    mutex.unlock();
}

void AndroidAudioDevice::resume() {
    mutex.lock();
    pauseRequest = 0;
    condition.signal();
    mutex.unlock();
}

void AndroidAudioDevice::flush() {
    mutex.lock();
    flushRequest = 1;
    condition.signal();
    mutex.unlock();
}

void AndroidAudioDevice::setStereoVolume(float left_volume, float right_volume) {
    mutex.lock();
    if (!updateVolume) {
        leftVolume = left_volume;
        rightVolume = right_volume;
        updateVolume = true;
    }
    condition.signal();
    mutex.unlock();
}

void AndroidAudioDevice::run() {
    uint8_t *next_buffer = nullptr;
    int next_buffer_index = 0;

    if (!abortRequest && !pauseRequest) {
        (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PLAYING);
    }

    while (true) {

        // 退出播放线程
        if (abortRequest) {
            break;
        }

        // 暂停
        if (pauseRequest) {
            continue;
        }

        // 获取缓冲队列状态
        SLAndroidSimpleBufferQueueState slState = {0};
        SLresult slRet = (*slBufferQueueItf)->GetState(slBufferQueueItf, &slState);
        if (slRet != SL_RESULT_SUCCESS) {
            ALOGE(TAG, "%s: slBufferQueueItf->GetState() failed\n", __func__);
            mutex.unlock();
        }

        // 判断暂停或者队列中缓冲区填满了
        mutex.lock();
        if (!abortRequest && (pauseRequest || slState.count >= OPENSLES_BUFFERS)) {
            while (!abortRequest && (pauseRequest || slState.count >= OPENSLES_BUFFERS)) {

                if (!pauseRequest) {
                    (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PLAYING);
                }
                condition.waitRelative(mutex, 10 * 1000000);
                slRet = (*slBufferQueueItf)->GetState(slBufferQueueItf, &slState);
                if (slRet != SL_RESULT_SUCCESS) {
                    ALOGE(TAG, "%s: slBufferQueueItf->GetState() failed\n", __func__);
                    mutex.unlock();
                }

                // pause
                if (pauseRequest) {
                    (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PAUSED);
                }
            }

            // play
            if (!abortRequest && !pauseRequest) {
                (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PLAYING);
            }
        }

        if (flushRequest) {
            (*slBufferQueueItf)->Clear(slBufferQueueItf);
            flushRequest = 0;
        }
        mutex.unlock();

        // 通过回调填充PCM数据
        mutex.lock();
        if (audioDeviceSpec.callback != nullptr) {
            next_buffer = buffer + next_buffer_index * bytes_per_buffer;
            next_buffer_index = (next_buffer_index + 1) % OPENSLES_BUFFERS;
            audioDeviceSpec.callback(audioDeviceSpec.userdata, next_buffer, bytes_per_buffer);
        }
        mutex.unlock();

        // 更新音量
        if (updateVolume) {
            if (slVolumeItf != nullptr) {
                SLmillibel level = getAmplificationLevel((leftVolume + rightVolume) / 2);
                SLresult result = (*slVolumeItf)->SetVolumeLevel(slVolumeItf, level);
                if (result != SL_RESULT_SUCCESS) {
                    ALOGE(TAG, "slVolumeItf->SetVolumeLevel failed %d", (int) result);
                }
            }
            updateVolume = false;
        }

        // 刷新缓冲区还是将数据入队缓冲区
        if (flushRequest) {
            (*slBufferQueueItf)->Clear(slBufferQueueItf);
            flushRequest = 0;
        } else {
            if (slPlayItf != nullptr) {
                (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PLAYING);
            }
            slRet = (*slBufferQueueItf)->Enqueue(slBufferQueueItf, next_buffer,
                                                 static_cast<SLuint32>(bytes_per_buffer));
            if (slRet == SL_RESULT_SUCCESS) {
                // do nothing
            } else if (slRet == SL_RESULT_BUFFER_INSUFFICIENT) {
                // don't retry, just pass through
                ALOGE(TAG, "SL_RESULT_BUFFER_INSUFFICIENT");
            } else {
                ALOGE(TAG, "slBufferQueueItf->Enqueue() = %d", (int) slRet);
                break;
            }
        }
    }

    if (slPlayItf) {
        (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_STOPPED);
    }
}

SLuint32 AndroidAudioDevice::getSLSampleRate(int sampleRate) {
    switch (sampleRate) {
        case 8000: {
            return SL_SAMPLINGRATE_8;
        }
        case 11025: {
            return SL_SAMPLINGRATE_11_025;
        }
        case 12000: {
            return SL_SAMPLINGRATE_12;
        }
        case 16000: {
            return SL_SAMPLINGRATE_16;
        }
        case 22050: {
            return SL_SAMPLINGRATE_22_05;
        }
        case 24000: {
            return SL_SAMPLINGRATE_24;
        }
        case 32000: {
            return SL_SAMPLINGRATE_32;
        }
        case 44100: {
            return SL_SAMPLINGRATE_44_1;
        }
        case 48000: {
            return SL_SAMPLINGRATE_48;
        }
        case 64000: {
            return SL_SAMPLINGRATE_64;
        }
        case 88200: {
            return SL_SAMPLINGRATE_88_2;
        }
        case 96000: {
            return SL_SAMPLINGRATE_96;
        }
        case 192000: {
            return SL_SAMPLINGRATE_192;
        }
        default: {
            return SL_SAMPLINGRATE_44_1;
        }
    }
}

SLmillibel AndroidAudioDevice::getAmplificationLevel(float volumeLevel) {
    if (volumeLevel < 0.00000001F) {
        return SL_MILLIBEL_MIN;
    }
    SLmillibel mb = lroundf(2000.0F * log10f(volumeLevel));
    if (mb < SL_MILLIBEL_MIN) {
        mb = SL_MILLIBEL_MIN;
    } else if (mb > 0) {
        mb = 0;
    }
    return mb;
}
