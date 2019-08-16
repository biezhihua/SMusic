#include "MacSurface.h"

int MacSurface::create() {
    ALOGD(MAC_SURFACE_TAG, __func__);

    if (!play) {
        return NEGATIVE(S_NULL);
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        ALOGD(MAC_SURFACE_TAG, "%s init sdl fail code = %s", __func__, SDL_GetError());
        return NEGATIVE(S_SDL_NOT_INIT);
    }

    Uint32 flags = SDL_WINDOW_HIDDEN;
    if (play->optionBrorderless) {
        flags |= SDL_WINDOW_BORDERLESS;
    } else {
        flags |= SDL_WINDOW_RESIZABLE;
    }

    window = SDL_CreateWindow(play->optionWindowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, play->optionDefaultWidth, play->optionDefaultHeight, flags);
    if (window == nullptr) {
        ALOGE(MAC_SURFACE_TAG, "%s create sdl window fail: %s", __func__, SDL_GetError());
        destroy();
        return NEGATIVE(S_SDL_NOT_CREATE_WINDOW);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        ALOGE(MAC_SURFACE_TAG, "%s failed to initialize a hardware accelerated renderer: %s", __func__, SDL_GetError());
        renderer = SDL_CreateRenderer(window, -1, 0);
    }
    if (renderer == nullptr) {
        ALOGE(MAC_SURFACE_TAG, "%s create renderer fail: %s", __func__, SDL_GetError());
        destroy();
        return NEGATIVE(S_SDL_NOT_CREATE_RENDERER);
    }

    if (!SDL_GetRendererInfo(renderer, &rendererInfo)) {
        ALOGD(MAC_SURFACE_TAG, "initialized %s renderer", rendererInfo.name);
    }

    if (!window || !renderer || !rendererInfo.num_texture_formats) {
        ALOGD(MAC_SURFACE_TAG, "%s failed to create window or renderer: %s", __func__, SDL_GetError());
        destroy();
    }
    return POSITIVE;
}

int MacSurface::destroy() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
    return POSITIVE;
}

void MacSurface::setWindowTitle(char *title) {

}

void MacSurface::setWindowSize(int width, int height, AVRational rational) {
    SDL_Rect rect;
    calculateDisplayRect(&rect, 0, 0, INT_MAX, height, width, height, rational);
    if (play) {
        play->optionDefaultWidth = rect.w;
        play->optionDefaultHeight = rect.h;
    }
}

void MacSurface::calculateDisplayRect(SDL_Rect *rect, int scrXLeft, int scrYTop,
                                      int scrWidth, int scrHeight,
                                      int picWidth, int picHeight,
                                      AVRational picSar
) {
    double aspectRatio;
    int width, height, x, y;

    if (picSar.num == 0) {
        aspectRatio = 0;
    } else {
        aspectRatio = av_q2d(picSar);
    }

    if (aspectRatio <= 0.0) {
        aspectRatio = 1.0;
    }

    aspectRatio *= (float) picWidth / (float) picHeight;

    /* XXX: we suppose the screen has a 1.0 pixel ratio */
    height = scrHeight;
    width = static_cast<int>(lrint(height * aspectRatio) & ~1);
    if (width > scrWidth) {
        width = scrWidth;
        height = static_cast<int>(lrint(width / aspectRatio) & ~1);
    }
    x = (scrWidth - width) / 2;
    y = (scrHeight - height) / 2;
    rect->x = scrXLeft + x;
    rect->y = scrYTop + y;
    rect->w = FFMAX(width, 1);
    rect->h = FFMAX(height, 1);

    ALOGD(MAC_SURFACE_TAG, "%s x = %d y = %d w = %d h = %d", __func__, rect->x, rect->y, rect->w, rect->h);
}
