#include "Audio.h"

Audio::Audio() = default;

Audio::~Audio() = default;

void Audio::setStream(Stream *stream) {
    Audio::stream = stream;
}
