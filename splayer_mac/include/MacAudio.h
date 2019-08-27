#ifndef SPLAYER_MAC_AUDIO_H
#define SPLAYER_MAC_AUDIO_H

#include "Audio.h"
#include "Error.h"
#include <SDL_audio.h>
#include <SDL.h>
#include "MacOptions.h"

extern "C" {
#include <libavutil/samplefmt.h>
}

#define MAC_AUDIO_TAG "MacAudio"

class MacAudio : public Audio {

private:

    SDL_Window *window;

    SDL_AudioDeviceID audioDev;

public:
    int create() override;

    int destroy() override;

    int openAudio(int64_t wantedChannelLayout, int wantedNbChannels, int wantedSampleRate, AudioParams *wantedAudioTarget) override;

    void pauseAudio() override;

    void maxAudio(uint8_t *stream, uint8_t *buf, int index, int length, int volume) override;
};


#endif //SPLAYER_MAC_AUDIO_H
