#include "Stream.h"

Stream::Stream() {
}

Stream::~Stream() {
}

void Stream::setSurface(Surface *surface) {
    Stream::surface = surface;
}

void Stream::setPlay(FFPlay *play) {
    Stream::play = play;
}
