#include "AudioDevice.h"

AudioDevice::AudioDevice() {

}

AudioDevice::~AudioDevice() {

}

int AudioDevice::open(AudioDeviceSpec *desired, AudioDeviceSpec *obtained) {
    return 0;
}

void AudioDevice::start() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
}

void AudioDevice::stop() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
}

void AudioDevice::pause() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
}

void AudioDevice::resume() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
}

void AudioDevice::flush() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
}

void AudioDevice::setStereoVolume(float left_volume, float right_volume) {

}

void AudioDevice::run() {
    // do nothing
}

int AudioDevice::create() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    return SUCCESS;
}

void AudioDevice::destroy() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }

}

void AudioDevice::setMute(bool mute) {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
}
