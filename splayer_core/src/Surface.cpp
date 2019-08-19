
#include <Surface.h>

#include "Surface.h"

Surface::Surface() {
    mutex = new Mutex();
}

Surface::~Surface() {
    delete mutex;
}

void Surface::setPlay(FFPlay *play) {
    Surface::play = play;
}

void Surface::setWindowSize(int width, int height, AVRational rational) {
}

void Surface::displayWindow() {

}

void Surface::displayVideoImage() {

}

Options *Surface::getOptions() const {
    return options;
}

void Surface::setOptions(Options *options) {
    Surface::options = options;
}

Stream *Surface::getStream() const {
    return stream;
}

void Surface::setStream(Stream *stream) {
    Surface::stream = stream;
}

void Surface::doExit() {
    if (play) {
        play->streamClose();
    }
    if (stream) {
        stream->destroy();
    }
    destroy();
}


