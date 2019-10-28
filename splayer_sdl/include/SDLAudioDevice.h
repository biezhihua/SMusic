
#ifndef SDL_AUDIODEVICE_H
#define SDL_AUDIODEVICE_H

#include <device/AudioDevice.h>
#include <SDL_audio.h>

class SDLAudioDevice : public AudioDevice {

    const char *const TAG = "[MP][SDL][AudioDevice]";

private:
    SDL_AudioDeviceID audioDev;

public:
    int open(AudioDeviceSpec *desired, AudioDeviceSpec *obtained) override;

    int create() override;

    void destroy() override;

    void start() override;

    void stop() override;

    void pause() override;

    void resume() override;

    void flush() override;

    void setStereoVolume(float left_volume, float right_volume) override;

    void run() override;

    SDL_AudioFormat getSDLFormat(AVSampleFormat format);

    AVSampleFormat getAVFormat(SDL_AudioFormat format);

};


#endif
