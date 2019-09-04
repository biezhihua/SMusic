#include "MacMediaSync.h"

void MacMediaSync::run() {
    bool quit = false;
    SDL_Event event;
    while (!quit) {
        resetRemainingTime();
        SDL_PumpEvents();
        while (!SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
            refreshVideo();
            SDL_PumpEvents();
        }
    }
}
