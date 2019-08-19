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
    if (options->borderLess) {
        flags |= SDL_WINDOW_BORDERLESS;
    } else {
        flags |= SDL_WINDOW_RESIZABLE;
    }

    window = SDL_CreateWindow(options->windowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, options->screenWidth, options->screenHeight, flags);
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
    options = nullptr;
    return POSITIVE;
}

void MacSurface::setWindowSize(int width, int height, AVRational rational) {
    SDL_Rect rect;
    calculateDisplayRect(&rect, 0, 0, width, height, width, height, rational);
    if (options) {
        options->screenWidth = rect.w;
        options->screenHeight = rect.h;
    }
    ALOGD(MAC_SURFACE_TAG, "%s x = %d y = %d w = %d h = %d", __func__, rect.x, rect.y, rect.w, rect.h);
}

void MacSurface::calculateDisplayRect(SDL_Rect *rect, int screenXLeft, int screenYTop,
                                      int screenWidth, int screenHeight,
                                      int pictureWidth, int pictureHeight,
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

    aspectRatio *= (float) pictureWidth / (float) pictureHeight;

    /* XXX: we suppose the screen has a 1.0 pixel ratio */
    height = screenHeight;
    width = static_cast<int>(lrint(height * aspectRatio) & ~1);
    if (width > screenWidth) {
        width = screenWidth;
        height = static_cast<int>(lrint(width / aspectRatio) & ~1);
    }
    x = (screenWidth - width) / 2;
    y = (screenHeight - height) / 2;
    rect->x = screenXLeft + x;
    rect->y = screenYTop + y;
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

//                if (event.type == SDL_MOUSEBUTTONDOWN) {
//                    if (event.button.button != SDL_BUTTON_RIGHT)
//                        break;
//                    x = event.button.x;
//                } else {
//                    if (!(event.motion.state & SDL_BUTTON_RMASK))
//                        break;
//                    x = event.motion.x;
//                }
//                if (seek_by_bytes || cur_stream->ic->duration <= 0) {
//                    uint64_t size = avio_size(cur_stream->ic->pb);
//                    stream_seek(cur_stream, size * x / cur_stream->width, 0, 1);
//                } else {
//                    int64_t ts;
//                    int ns, hh, mm, ss;
//                    int tns, thh, tmm, tss;
//                    tns = cur_stream->ic->duration / 1000000LL;
//                    thh = tns / 3600;
//                    tmm = (tns % 3600) / 60;
//                    tss = (tns % 60);
//                    frac = x / cur_stream->width;
//                    ns = frac * tns;
//                    hh = ns / 3600;
//                    mm = (ns % 3600) / 60;
//                    ss = (ns % 60);
//                    av_log(NULL, AV_LOG_INFO,
//                           "Seek to %2.0f%% (%2d:%02d:%02d) of total duration (%2d:%02d:%02d)       \n", frac * 100,
//                           hh, mm, ss, thh, tmm, tss);
//                    ts = frac * cur_stream->ic->duration;
//                    if (cur_stream->ic->start_time != AV_NOPTS_VALUE)
//                        ts += cur_stream->ic->start_time;
//                    stream_seek(cur_stream, ts, 0, 0);
//                }
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
        case SDLK_RIGHT:
        case SDLK_UP:
        case SDLK_DOWN:
            break;
        default:
            break;
    }
}

void MacSurface::displayWindow() {
    if (window && options) {
        SDL_SetWindowTitle(window, options->windowTitle);
        SDL_SetWindowSize(window, options->screenWidth, options->screenHeight);
        SDL_SetWindowPosition(window, options->screenLeft, options->screenTop);
        if (options->isFullScreen) {
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
        SDL_ShowWindow(window);
        // Query Real Window Size And Update
        SDL_GetWindowSize(window, &options->screenWidth, &options->screenHeight);
        ALOGD(MAC_SURFACE_TAG, "%s windowTitle = %s screenWidth = %d screenHeight = %d screenLeft = %d screenTop = %d", __func__,
              options->windowTitle,
              options->screenWidth, options->screenHeight,
              options->screenLeft, options->screenTop
        );
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

        lastFrame = videoState->videoFrameQueue.peekCurrentShownFrame();

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

void MacSurface::doExit() {
    Surface::doExit();
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
