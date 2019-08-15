#include "MacSurface.h"

int MacSurface::create() {
//    if (!play) {
//        return NEGATIVE(S_NULL);
//    }
//    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
//        ALOGD(MAC_SURFACE_TAG, "%s init sdl fail SDL_Init", __func__);
//        return NEGATIVE(S_SDL_NOT_INIT);
//    }
//    int flags = SDL_WINDOW_HIDDEN;
//    if (play->borderless) {
//        flags |= SDL_WINDOW_BORDERLESS;
//    } else {
//        flags |= SDL_WINDOW_RESIZABLE;
//    }
//    window = SDL_CreateWindow(program_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, default_width, default_height, flags);

    return POSITIVE;
}

int MacSurface::destroy() {
    return POSITIVE;
}
