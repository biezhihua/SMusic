#include <SDLAudioDevice.h>

int SDLAudioDevice::open(AudioDeviceSpec *desired, AudioDeviceSpec *obtained) {
    SDL_AudioSpec desiredSpec, obtainedSpec;
    desiredSpec.channels = desired->channels;
    desiredSpec.size = desired->size;
    desiredSpec.format = getSDLFormat(desired->format);
    desiredSpec.freq = desired->sampleRate;
    desiredSpec.userdata = desired->userdata;
    desiredSpec.samples = desired->samples;
    desiredSpec.callback = desired->callback;
    audioDev = SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, &obtainedSpec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
    if (audioDev) {
        obtained->channels = obtainedSpec.channels;
        obtained->size = obtainedSpec.size;
        obtained->format = getAVFormat(obtainedSpec.format);
        obtained->sampleRate = obtainedSpec.freq;
        obtained->userdata = obtainedSpec.userdata;
        obtained->samples = obtainedSpec.samples;
        obtained->callback = obtainedSpec.callback;
        return SUCCESS;
    }
    return ERROR_AUDIO_OPEN;
}

int SDLAudioDevice::create() {
    AudioDevice::create();
    return SUCCESS;
}

void SDLAudioDevice::destroy() {
    AudioDevice::destroy();
}

void SDLAudioDevice::start() {
    SDL_PauseAudioDevice(audioDev, 0);
}

void SDLAudioDevice::stop() {
    AudioDevice::stop();
}

void SDLAudioDevice::pause() {
    AudioDevice::pause();
}

void SDLAudioDevice::resume() {
    AudioDevice::resume();
}

void SDLAudioDevice::flush() {
    AudioDevice::flush();
}

void SDLAudioDevice::setStereoVolume(float left_volume, float right_volume) {
    AudioDevice::setStereoVolume(left_volume, right_volume);
}

void SDLAudioDevice::run() {
    AudioDevice::run();
}

SDL_AudioFormat SDLAudioDevice::getSDLFormat(AVSampleFormat format) {
    switch (format) {
        case AV_SAMPLE_FMT_U8:
            return AUDIO_U8;
        case AV_SAMPLE_FMT_S16:
            return AUDIO_S16;
    }
    return AUDIO_S16SYS;
}

AVSampleFormat SDLAudioDevice::getAVFormat(SDL_AudioFormat format) {
    switch (format) {
        case AUDIO_U8:
            return AV_SAMPLE_FMT_U8;
        case AUDIO_S16:
            return AV_SAMPLE_FMT_S16;
    }
    return AV_SAMPLE_FMT_S16;
}
