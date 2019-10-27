
#include <device/AudioDevice.h>

#include "device/AudioDevice.h"

AudioDevice::AudioDevice() {

}

AudioDevice::~AudioDevice() {

}

int AudioDevice::open(AudioDeviceSpec *desired, AudioDeviceSpec *obtained) {
    return 0;
}

void AudioDevice::start() {
    if (DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
}

void AudioDevice::stop() {
    if (DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
}

void AudioDevice::pause() {
    if (DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
}

void AudioDevice::resume() {
    if (DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
}

void AudioDevice::flush() {
    if (DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
}

void AudioDevice::setStereoVolume(float left_volume, float right_volume) {

}

void AudioDevice::run() {
    // do nothing
}

int AudioDevice::create() {
    if (DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    return SUCCESS;
}

void AudioDevice::destroy() {
    if (DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }

}
