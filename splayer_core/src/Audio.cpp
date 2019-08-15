#include "Audio.h"

Audio::Audio() = default;

Audio::~Audio() = default;

void Audio::setPlay(FFPlay *play) {
    Audio::play = play;
}
