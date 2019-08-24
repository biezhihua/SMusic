
#include <Audio.h>

#include "Audio.h"

Audio::Audio() = default;

Audio::~Audio() = default;

void Audio::setStream(Stream *stream) {
    Audio::stream = stream;
}

int Audio::openAudio(int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, AudioParams *audio_hw_params) {
    return 0;
}
