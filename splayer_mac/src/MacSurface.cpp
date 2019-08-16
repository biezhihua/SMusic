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

    Uint32 flags = SDL_WINDOW_SHOWN;
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

int MacSurface::eventLoop() {
    bool quit = false;
    SDL_Event event;
    while (!quit) {
        SDL_PumpEvents();
        while (!SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
            if (play && !play->optionCursorHidden && av_gettime_relative() - play->optionCursorLastShown > CURSOR_HIDE_DELAY) {
                SDL_ShowCursor(0);
                play->optionCursorHidden = 1;
            }
            SDL_PumpEvents();
        }
        switch (event.type) {
            case SDL_KEYDOWN:
                if (isQuitKey(event)) {
                    quit = true;
                    break;
                }
                if (isHaveWindow()) {
                    continue;
                }
                doKeySystem(event);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEMOTION:
            case SDL_WINDOWEVENT:
                break;
            case SDL_QUIT:
                quit = true;
                break;
            default:
                break;
        }
    }
    return POSITIVE;
}

bool MacSurface::isQuitKey(const SDL_Event &event) const {
    return event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q;
}

bool MacSurface::isHaveWindow() const {
    // If we don't yet have a window, skip all key events, because read_thread might still be initializing...
    return play && play->getVideoState() && play->getVideoState()->width;
}

void MacSurface::doKeySystem(const SDL_Event &event) const {
    switch (event.key.keysym.sym) {
        case SDLK_f:
            break;
        case SDLK_p:
        case SDLK_SPACE:
            break;
        case SDLK_m:
            break;
        case SDLK_KP_MULTIPLY:
        case SDLK_0:
            break;
        case SDLK_KP_DIVIDE:
        case SDLK_9:
            break;
        case SDLK_s: // S: Step to next frame
            break;
        case SDLK_a:
            break;
        case SDLK_v:
            break;
        case SDLK_c:
            break;
        case SDLK_t:
            break;
        case SDLK_w:
            break;
        case SDLK_PAGEUP:
            break;
        case SDLK_PAGEDOWN:
            break;
        case SDLK_LEFT:
        case SDLK_RIGHT:
        case SDLK_UP:
        case SDLK_DOWN:
            break;
        default:
            break;
    }
}
