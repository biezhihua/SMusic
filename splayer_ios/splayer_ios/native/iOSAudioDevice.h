#ifndef SPLAYER_IOS_AUDIODEVICE_H
#define SPLAYER_IOS_AUDIODEVICE_H

#include "AudioDevice.h"

class iOSAudioDevice : public AudioDevice {

public:

    iOSAudioDevice();

    ~iOSAudioDevice() override;

    int create() override;

    void destroy() override;

    int open(AudioDeviceSpec *desired, AudioDeviceSpec *obtained) override;

    void start() override;

    void stop() override;

    void pause() override;

    void resume() override;

    void flush() override;

    void setStereoVolume(float left_volume, float right_volume) override;

    void setMute(bool mute) override;

    void run() override;
};


#endif
