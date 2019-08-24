#ifndef SPLAYER_COMMAND_MACAOUT_H
#define SPLAYER_COMMAND_MACAOUT_H

#include "Audio.h"
#include "Error.h"
#include <SDL_audio.h>

#define MAC_AUDIO_TAG "MacAudio"

class MacAudio : public Audio {

private:
    SDL_AudioDeviceID audioDev;

public:
    int create() override;

    int destroy() override;

    int openAudio(int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, AudioParams *audio_hw_params) override;

    void audioCallback(Uint8 *stream, int len);
};


#endif //SPLAYER_COMMAND_MACAOUT_H
