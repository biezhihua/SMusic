#include <SDLAudioDevice.h>

int SDLAudioDevice::open(const AudioDeviceSpec *desired, AudioDeviceSpec *obtained) {
    SDL_AudioSpec wantedSpec, spec;
    wantedSpec.channels = desired->channels;
    wantedSpec.size = desired->size;
    wantedSpec.format = getSDLFormat(desired->format);
    wantedSpec.freq = desired->freq;
    wantedSpec.userdata = desired->userdata;
    wantedSpec.samples = desired->samples;
    wantedSpec.callback = desired->callback;
    audioDev = SDL_OpenAudioDevice(nullptr, 0, &wantedSpec, &spec,
                                   SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
    if (audioDev) {
        obtained->channels = spec.channels;
        obtained->size = spec.size;
        obtained->format = getAVFormat(spec.format);
        obtained->freq = spec.freq;
        obtained->userdata = spec.userdata;
        obtained->samples = spec.samples;
        obtained->callback = spec.callback;
        return SUCCESS;
    }
    return ERROR_AUDIO_OPEN;
}

void SDLAudioDevice::create() {
    AudioDevice::create();
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
