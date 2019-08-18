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
    if (options) {
        options->screenWidth = width;
        options->screenHeight = height;
    }
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


