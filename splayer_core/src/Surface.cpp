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

void Surface::setWindowTitle(char *title) {

}


