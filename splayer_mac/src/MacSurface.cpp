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
    auto *mapOptions = dynamic_cast<MacOptions *>(options);
    if (mapOptions->borderLess) {
        flags |= SDL_WINDOW_BORDERLESS;
    } else {
        flags |= SDL_WINDOW_RESIZABLE;
    }

    if (!SDL_getenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE")) {
        SDL_setenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE", "1", 1);
    }

    window = SDL_CreateWindow(options->windowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, options->defaultWidth, options->defaultHeight, flags);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

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
        return NEGATIVE(S_SDL_NOT_INIT);
    }

    return POSITIVE;
}

int MacSurface::destroy() {
    if (videoTexture) {
        SDL_DestroyTexture(videoTexture);
        videoTexture = nullptr;
    }
    if (subtitleTexture) {
        SDL_DestroyTexture(subtitleTexture);
        subtitleTexture = nullptr;
    }
    if (visTexture) {
        SDL_DestroyTexture(visTexture);
        visTexture = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_Quit();
    rendererInfo = {nullptr};
    lastMouseLeftClick = 0;
    options = nullptr;
    return POSITIVE;
}

int MacSurface::eventLoop() {
    bool quit = false;
    SDL_Event event;
    while (!quit && play) {
        play->remainingTime = 0.0F;
        SDL_PumpEvents();
        while (!SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
            hideCursor();
            if (play) {
                play->refresh();
            }
            SDL_PumpEvents();
        }
        switch (event.type) {
            case SDL_KEYDOWN:
                if (isQuitKey(event)) {
                    doExit();
                    quit = true;
                    break;
                }
                if (isNotHaveWindow()) {
                    continue;
                }
                doKeySystem(event);
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (play && event.button.button == SDL_BUTTON_LEFT && !IS_NEGATIVE(isFullScreenClick())) {
                    toggleFullScreen();
                    play->forceRefresh();
                }
            case SDL_MOUSEMOTION:
                showCursor();
                break;
            case SDL_WINDOWEVENT:
                doWindowEvent(event);
                break;
            case SDL_QUIT:
                doExit();
                quit = true;
                break;
            default:
                break;
        }
    }
    return POSITIVE;
}

void MacSurface::hideCursor() const {
    if (options) {
        auto *macOptions = (MacOptions *) (options);
        if (!macOptions->cursorHidden && (av_gettime_relative() - macOptions->cursorLastShown) > CURSOR_HIDE_DELAY) {
            SDL_ShowCursor(0);
            macOptions->cursorHidden = 1;
        }
    }
}

void MacSurface::showCursor() const {
    if (options) {
        auto *macOptions = (MacOptions *) (options);
        if (macOptions->cursorHidden) {
            SDL_ShowCursor(1);
            macOptions->cursorHidden = 0;
        }
        macOptions->cursorLastShown = av_gettime_relative();
    }
}

void MacSurface::doWindowEvent(const SDL_Event &event) {
    ALOGD(MAC_SURFACE_TAG, "%s width = %d height = %d ", __func__, event.window.data1, event.window.data2);
    switch (event.window.event) {
        case SDL_WINDOWEVENT_RESIZED:
            if (options && play) {
                options->screenWidth = play->getVideoState()->width = event.window.data1;
                options->screenHeight = play->getVideoState()->height = event.window.data2;
                if (videoTexture) {
                    SDL_DestroyTexture(videoTexture);
                    videoTexture = nullptr;
                }
            }
        case SDL_WINDOWEVENT_EXPOSED:
            if (play) {
                play->forceRefresh();
            }
        default:
            break;
    }
}

bool MacSurface::isQuitKey(const SDL_Event &event) const {
    return event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q;
}

bool MacSurface::isNotHaveWindow() const {
    // If we don't yet have a window, skip all key events, because read_thread might still be initializing...
    return !(play && play->getVideoState() && play->getVideoState()->width);
}

void MacSurface::doKeySystem(const SDL_Event &event) const {
    switch (event.key.keysym.sym) {
        case SDLK_f:
            if (play) {
                toggleFullScreen();
                play->forceRefresh();
            }
            break;
        case SDLK_p:
        case SDLK_SPACE:
            if (play) {
                play->togglePause();
            }
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
            if (play) {
                play->setupToNextFrame();
            }
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
            if (options) {
                doSeek(options->seekInterval != 0 ? -options->seekInterval : -10.0);
            }
            break;
        case SDLK_RIGHT:
            if (options) {
                doSeek(options->seekInterval != 0 ? options->seekInterval : 10.0);
            }
            break;
        case SDLK_UP:
            doSeek(-60.0f);
            break;
        case SDLK_DOWN:
            doSeek(60.0f);
            break;
        default:
            break;
    }
}

void MacSurface::displayWindow(int width, int height) {
    if (window && options) {
        SDL_SetWindowTitle(window, options->windowTitle);
        SDL_SetWindowSize(window, width, height);
        SDL_SetWindowPosition(window, options->screenLeft, options->screenTop);
        if (options->isFullScreen) {
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
        SDL_ShowWindow(window);
        ALOGD(MAC_SURFACE_TAG, "%s windowTitle = %s defaultWidth = %d defaultHeight = %d screenLeft = %d screenTop = %d", __func__,
              options->windowTitle,
              width, height,
              options->screenLeft, options->screenTop
        );
    }
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
                                         frame->data[2], frame->linesize[2]) >= 0) {
                    return POSITIVE;
                } else {
                    return NEGATIVE(S_NOT_UPDATE_YUV_TEXTURE);
                }
            } else if (frame->linesize[0] < 0 && frame->linesize[1] < 0 && frame->linesize[2] < 0) {
                if (SDL_UpdateYUVTexture(videoTexture, nullptr,
                                         frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0],
                                         frame->data[1] + frame->linesize[1] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[1],
                                         frame->data[2] + frame->linesize[2] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[2]) >= 0) {
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
                if (SDL_UpdateTexture(videoTexture, nullptr, frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0]) >= 0) {
                    return POSITIVE;
                } else {
                    return NEGATIVE(S_NOT_UPDATE_TEXTURE);
                }
            } else {
                if (SDL_UpdateTexture(videoTexture, nullptr, frame->data[0], frame->linesize[0]) >= 0) {
                    return POSITIVE;
                } else {
                    return NEGATIVE(S_NOT_UPDATE_TEXTURE);
                }
            }
            break;
    }
    return POSITIVE;
}

void MacSurface::displayVideoImageBefore() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void MacSurface::displayVideoImageAfter(Frame *lastFrame, Rect *rect) {
    if (play && play->getVideoState() && lastFrame) {
        SDL_Rect sdlRect;
        if (rect) {
            sdlRect.x = rect->x;
            sdlRect.y = rect->y;
            sdlRect.w = rect->w;
            sdlRect.h = rect->h;
        }
        setYuvConversionMode(lastFrame->frame);
        SDL_RenderCopyEx(renderer, videoTexture, nullptr, &sdlRect, 0, nullptr, lastFrame->flipVertical ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE);
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
            ALOGD(MAC_SURFACE_TAG, "%s mode = %s", __func__, "SDL_YUV_CONVERSION_JPEG");
        } else if (frame->colorspace == AVCOL_SPC_BT709) {
            mode = SDL_YUV_CONVERSION_BT709;
            ALOGD(MAC_SURFACE_TAG, "%s mode = %s", __func__, "SDL_YUV_CONVERSION_BT709");
        } else if (frame->colorspace == AVCOL_SPC_BT470BG || frame->colorspace == AVCOL_SPC_SMPTE170M || frame->colorspace == AVCOL_SPC_SMPTE240M) {
            mode = SDL_YUV_CONVERSION_BT601;
            ALOGD(MAC_SURFACE_TAG, "%s mode = %s", __func__, "SDL_YUV_CONVERSION_BT601");
        }
    }
    SDL_SetYUVConversionMode(mode);
#endif
}

void MacSurface::setSdlBlendMode(int format, SDL_BlendMode *sdlBlendMode) {
    *sdlBlendMode = SDL_BLENDMODE_NONE;
    if (format == AV_PIX_FMT_RGB32 || format == AV_PIX_FMT_RGB32_1 || format == AV_PIX_FMT_BGR32 || format == AV_PIX_FMT_BGR32_1) {
        *sdlBlendMode = SDL_BLENDMODE_BLEND;
    }
}

void MacSurface::setSdlPixelFormat(int format, Uint32 *sdlPixelFormat) {
    *sdlPixelFormat = SDL_PIXELFORMAT_UNKNOWN;
    int size = FF_ARRAY_ELEMS(SDL_TEXTURE_FORMAT_MAP) - 1;
    for (int i = 0; i < size; i++) {
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

void MacSurface::doExit() {
    if (mediaPlayer) {
        mediaPlayer->destroy();
    }
    exit(0);
}

void MacSurface::toggleFullScreen() const {
    if (options && window) {
        options->isFullScreen = !options->isFullScreen;
        SDL_SetWindowFullscreen(window, options->isFullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    }
}

int MacSurface::isFullScreenClick() {
    int ret = NEGATIVE(S_ERROR);
    if ((av_gettime_relative() - lastMouseLeftClick) <= 5 * 100000) {
        lastMouseLeftClick = 0;
        ret = POSITIVE;
    } else {
        lastMouseLeftClick = av_gettime_relative();
    }
    return ret;
}

void MacSurface::doSeek(double increment) const {
    double pos;
    if (options && play && play->getVideoState()) {
        VideoState *videoState = play->getVideoState();
        if (options->seekByBytes) {
            pos = -1;
            if (pos < 0 && videoState->videoStreamIndex >= 0) {
                pos = videoState->videoFrameQueue.lastPos();
            }
            if (pos < 0 && videoState->audioStreamIndex >= 0) {
                pos = videoState->audioFrameQueue.lastPos();
            }
            if (pos < 0) {
                pos = avio_tell(videoState->formatContext->pb);
            }
            if (videoState->formatContext->bit_rate) {
                increment *= videoState->formatContext->bit_rate / 8.0;
            } else {
                increment *= 180000.0;
            }
            pos += increment;
            play->streamSeek((int64_t) pos, (int64_t) increment, 1);
        } else {
            pos = play->getMasterClock();
            if (isnan(pos)) {
                pos = (double) videoState->seekPos / AV_TIME_BASE;
            }
            pos += increment;
            if (videoState->formatContext->start_time != AV_NOPTS_VALUE && pos < videoState->formatContext->start_time / (double) AV_TIME_BASE) {
                pos = videoState->formatContext->start_time / (double) AV_TIME_BASE;
            }
            play->streamSeek((int64_t) (pos * AV_TIME_BASE), (int64_t) (increment * AV_TIME_BASE), 0);
        }
    }
}
