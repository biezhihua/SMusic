#include "Event.h"

Event::Event() = default;

Event::~Event() = default;

int Event::create() {
    return POSITIVE;
}

int Event::destroy() {
    return POSITIVE;
}

void Event::setStream(Stream *stream) {
    Event::stream = stream;
}

void Event::setOptions(Options *options) {
    Event::options = options;
}

void Event::setMediaPlayer(MediaPlayer *mediaPlayer) {
    Event::mediaPlayer = mediaPlayer;
}

void Event::setSurface(Surface *surface) {
    Event::surface = surface;
}
