#ifndef SPLAYER_COMMAND_MACAOUT_H
#define SPLAYER_COMMAND_MACAOUT_H

#include "Audio.h"
#include "Error.h"
#include <SDL_audio.h>

extern "C" {
#include <libavutil/samplefmt.h>
}

#define MAC_AUDIO_TAG "MacAudio"

class MacAudio : public Audio {

private:
    SDL_AudioDeviceID audioDev;

public:
    int create() override;

    int destroy() override;

    int openAudio(int64_t wantedChannelLayout, int wantedNbChannels, int wantedSampleRate, AudioParams *audioHwParams) override;

    void audioCallback(Uint8 *stream, int len);

    void pauseAudio() override;

    void update_sample_display(short *samples, int samples_size);
};


#endif //SPLAYER_COMMAND_MACAOUT_H
