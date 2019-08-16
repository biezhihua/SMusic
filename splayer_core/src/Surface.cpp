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

void Surface::openWindow(int width, int height) {

}


