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

int Stream::create() {
    avformat_network_init();
    return POSITIVE;
}

int Stream::destroy() {
    avformat_network_deinit();
    return POSITIVE;
}
