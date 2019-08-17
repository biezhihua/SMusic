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
    while (!quit && play) {
        play->remainingTime = 0.0F;
        SDL_PumpEvents();
        while (!SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
            if (play && !play->optionCursorHidden && av_gettime_relative() - play->optionCursorLastShown > CURSOR_HIDE_DELAY) {
                SDL_ShowCursor(0);
                play->optionCursorHidden = 1;
            }
            if (play) {
                play->refresh();
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

void MacSurface::displayWindow(int width, int height) {
    if (window && play) {
        SDL_SetWindowTitle(window, play->optionWindowTitle);
        SDL_SetWindowSize(window, width, height);
        SDL_SetWindowPosition(window, play->optionScreenLeft, play->optionScreenTop);
        if (play->optionIsFullScreen) {
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
        SDL_ShowWindow(window);
    }
}

void MacSurface::displayVideoImage() {
    Frame *lastFrame;
    Frame *sp = nullptr;
    SDL_Rect rect;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (play && play->getVideoState()) {
        VideoState *videoState = play->getVideoState();

        lastFrame = videoState->videoFrameQueue.peekLast();

        if (videoState->subtitleStream) {
            // TODO
        }

        calculateDisplayRect(&rect,
                             videoState->xLeft, videoState->yTop,
                             videoState->width, videoState->height,
                             lastFrame->width, lastFrame->height,
                             lastFrame->sampleAspectRatio);

        if (!lastFrame->uploaded) {
            if (!uploadTexture(lastFrame->frame, videoState->imgConvertCtx)) {
                return;
            }
            lastFrame->uploaded = 1;
            lastFrame->flipVertical = lastFrame->frame->linesize[0] < 0;
        }
        setYuvConversionMode(lastFrame->frame);
        SDL_RenderCopyEx(renderer, videoTexture, nullptr, &rect, 0, nullptr, lastFrame->flipVertical ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE);
        setYuvConversionMode(nullptr);
    }

    SDL_RenderPresent(renderer);
}

void MacSurface::setYuvConversionMode(AVFrame *frame) {
#if SDL_VERSION_ATLEAST(2, 0, 8)
    SDL_YUV_CONVERSION_MODE mode = SDL_YUV_CONVERSION_AUTOMATIC;
    if (frame && (frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUYV422 || frame->format == AV_PIX_FMT_UYVY422)) {
        if (frame->color_range == AVCOL_RANGE_JPEG) {
            mode = SDL_YUV_CONVERSION_JPEG;
        } else if (frame->colorspace == AVCOL_SPC_BT709) {
            mode = SDL_YUV_CONVERSION_BT709;
        } else if (frame->colorspace == AVCOL_SPC_BT470BG || frame->colorspace == AVCOL_SPC_SMPTE170M || frame->colorspace == AVCOL_SPC_SMPTE240M) {
            mode = SDL_YUV_CONVERSION_BT601;
        }
    }
    SDL_SetYUVConversionMode(mode);
#endif
}

int MacSurface::uploadTexture(AVFrame *frame, SwsContext *convertContext) {
    Uint32 sdlPixelFormat;
    SDL_BlendMode sdlBlendMode;

    setSdlPixelFormat(frame->format, &sdlPixelFormat);
    setSdlBlendMode(frame->format, &sdlBlendMode);

    Uint32 format = sdlPixelFormat == SDL_PIXELFORMAT_UNKNOWN ? SDL_PIXELFORMAT_ARGB8888 : sdlPixelFormat;
    if (!reallocTexture(&videoTexture, format, frame->width, frame->height, sdlBlendMode, 0)) {
        return NEGATIVE(S_NOT_REALLOC_TEXTURE);
    }

    switch (sdlPixelFormat) {
        case SDL_PIXELFORMAT_UNKNOWN:
            /* This should only happen if we are not using avfilter... */
            convertContext = sws_getCachedContext(convertContext,
                                                  frame->width, frame->height, (AVPixelFormat) frame->format,
                                                  frame->width, frame->height,
                                                  AV_PIX_FMT_BGRA, swsFlags, nullptr, nullptr, nullptr);
            if (convertContext != nullptr) {
                uint8_t *pixels[4];
                int pitch[4];
                if (!SDL_LockTexture(videoTexture, nullptr, (void **) pixels, pitch)) {
                    sws_scale(convertContext, (const uint8_t *const *) frame->data, frame->linesize, 0, frame->height, pixels, pitch);
                    SDL_UnlockTexture(videoTexture);
                }
            } else {
                ALOGE(MAC_SURFACE_TAG, "%s Cannot initialize the conversion context", __func__);
                return NEGATIVE(S_NOT_INIT_CONVERSION_CONTEXT);
            }
            break;
        case SDL_PIXELFORMAT_IYUV:
            if (frame->linesize[0] > 0 && frame->linesize[1] > 0 && frame->linesize[2] > 0) {
                if (SDL_UpdateYUVTexture(videoTexture, nullptr,
                                         frame->data[0], frame->linesize[0],
                                         frame->data[1], frame->linesize[1],
                                         frame->data[2], frame->linesize[2])) {
                    return POSITIVE;
                } else {
                    return NEGATIVE(S_NOT_UPDATE_YUV_TEXTURE);
                }
            } else if (frame->linesize[0] < 0 && frame->linesize[1] < 0 && frame->linesize[2] < 0) {
                if (SDL_UpdateYUVTexture(videoTexture, nullptr,
                                         frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0],
                                         frame->data[1] + frame->linesize[1] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[1],
                                         frame->data[2] + frame->linesize[2] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[2])) {
                    return POSITIVE;
                } else {
                    return NEGATIVE(S_NOT_UPDATE_YUV_TEXTURE);
                }
            } else {
                ALOGE(MAC_SURFACE_TAG, "%s Cannot initialize the conversion context", __func__);
                return NEGATIVE(S_NOT_SUPPORT_LINESIZES);
            }
            break;
        default:
            if (frame->linesize[0] < 0) {
                if (SDL_UpdateTexture(videoTexture, nullptr, frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0])) {
                    return POSITIVE;
                } else {
                    return NEGATIVE(S_NOT_UPDATE_TEXTURE);
                }
            } else {
                if (SDL_UpdateTexture(videoTexture, nullptr, frame->data[0], frame->linesize[0])) {
                    return POSITIVE;
                } else {
                    return NEGATIVE(S_NOT_UPDATE_TEXTURE);
                }
            }
            break;
    }
    return POSITIVE;
}

void MacSurface::setSdlBlendMode(int format, SDL_BlendMode *sdlBlendMode) {
    *sdlBlendMode = SDL_BLENDMODE_NONE;
    if (format == AV_PIX_FMT_RGB32 || format == AV_PIX_FMT_RGB32_1 || format == AV_PIX_FMT_BGR32 || format == AV_PIX_FMT_BGR32_1) {
        *sdlBlendMode = SDL_BLENDMODE_BLEND;
    }
}

void MacSurface::setSdlPixelFormat(int format, Uint32 *sdlPixelFormat) {
    *sdlPixelFormat = SDL_PIXELFORMAT_UNKNOWN;
    for (int i = 0; i < FF_ARRAY_ELEMS(SDL_TEXTURE_FORMAT_MAP) - 1; i++) {
        if (format == SDL_TEXTURE_FORMAT_MAP[i].format) {
            *sdlPixelFormat = static_cast<Uint32>(SDL_TEXTURE_FORMAT_MAP[i].textureFormat);
            return;
        }
    }
}

int MacSurface::reallocTexture(SDL_Texture **texture, Uint32 newFormat, int newWidth, int newHeight, SDL_BlendMode blendMode, int initTexture) {
    Uint32 format;
    int access;
    int width;
    int height;
    if (!*texture || SDL_QueryTexture(*texture, &format, &access, &width, &height) < 0 || newWidth != width || newHeight != height || newFormat != format) {
        void *pixels;
        int pitch;
        if (*texture) {
            SDL_DestroyTexture(*texture);
        }
        if (!(*texture = SDL_CreateTexture(renderer, newFormat, SDL_TEXTUREACCESS_STREAMING, newWidth, newHeight))) {
            return NEGATIVE(S_NOT_CREATE_TEXTURE);
        }
        if (SDL_SetTextureBlendMode(*texture, blendMode) < 0) {
            return NEGATIVE(S_NOT_SET_BLEND_MODE);
        }
        if (initTexture) {
            if (SDL_LockTexture(*texture, nullptr, &pixels, &pitch) < 0) {
                return NEGATIVE(S_NOT_LOCK_TEXTURE);;
            }
            memset(pixels, 0, pitch * newHeight);
            SDL_UnlockTexture(*texture);
        }
        ALOGD(MAC_SURFACE_TAG, "%s Created %dx%d texture with %s", __func__, newWidth, newHeight, SDL_GetPixelFormatName(newFormat));
    }
    return POSITIVE;
}

