#include "iOSAudioDevice.h"

iOSAudioDevice::~iOSAudioDevice() {

}

int iOSAudioDevice::create() {
    return AudioDevice::create();
}

void iOSAudioDevice::destroy() {
    AudioDevice::destroy();
}

int iOSAudioDevice::open(AudioDeviceSpec *desired, AudioDeviceSpec *obtained) {
    return AudioDevice::open(desired, obtained);
}

void iOSAudioDevice::start() {
    AudioDevice::start();
}

void iOSAudioDevice::stop() {
    AudioDevice::stop();
}

void iOSAudioDevice::pause() {
    AudioDevice::pause();
}

void iOSAudioDevice::resume() {
    AudioDevice::resume();
}

void iOSAudioDevice::flush() {
    AudioDevice::flush();
}

void iOSAudioDevice::setStereoVolume(float left_volume, float right_volume) {
    AudioDevice::setStereoVolume(left_volume, right_volume);
}

void iOSAudioDevice::setMute(bool mute) {
    AudioDevice::setMute(mute);
}

void iOSAudioDevice::run() {
    AudioDevice::run();
}

iOSAudioDevice::iOSAudioDevice() {

}
