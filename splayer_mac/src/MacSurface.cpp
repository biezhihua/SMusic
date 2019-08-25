#include "MacSurface.h"

int MacSurface::create() {
    ALOGD(MAC_SURFACE_TAG, __func__);

    if (!stream) {
        return NEGATIVE(S_NULL);
    }

    auto *macOptions = dynamic_cast<MacOptions *>(options);


    if (macOptions->displayDisable) {
        macOptions->videoDisable = 1;
    }

    unsigned int initFlags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;

    if (macOptions->audioDisable) {
        initFlags &= ~SDL_INIT_AUDIO;
    } else {
        /* Try to work around an occasional ALSA buffer underflow issue when the
         * period size is NPOT due to ALSA resampling by forcing the buffer size. */
        if (!SDL_getenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE")) {
            SDL_setenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE", "1", 1);
        }
    }

    if (macOptions->displayDisable) {
        initFlags &= ~SDL_INIT_VIDEO;
    }

    if (SDL_Init(initFlags) != 0) {
        ALOGD(MAC_SURFACE_TAG, "%s init sdl fail code = %s", __func__, SDL_GetError());
        doExit();
        return NEGATIVE(S_NO_SDL_INIT);
    }

    if (!macOptions->displayDisable) {

        Uint32 windowFlags = SDL_WINDOW_HIDDEN;

        if (macOptions->alwaysOntop) {
#if SDL_VERSION_ATLEAST(2, 0, 5)
            windowFlags |= SDL_WINDOW_ALWAYS_ON_TOP;
#else
            ALOGE(MAC_SURFACE_TAG, "%s Your SDL version doesn't support SDL_WINDOW_ALWAYS_ON_TOP. Feature will be inactive.", __func__);
#endif
        }

        if (macOptions->borderLess) {
            windowFlags |= SDL_WINDOW_BORDERLESS;
        } else {
            windowFlags |= SDL_WINDOW_RESIZABLE;
        }

        window = SDL_CreateWindow(options->videoTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, options->videoWidth, options->videoHeight, windowFlags);

        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

        if (window == nullptr) {
            ALOGE(MAC_SURFACE_TAG, "%s create sdl window fail: %s", __func__, SDL_GetError());
            destroy();
            return NEGATIVE(S_NO_SDL_CREATE_WINDOW);
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (renderer == nullptr) {
            ALOGE(MAC_SURFACE_TAG, "%s failed to initialize a hardware accelerated renderer: %s", __func__, SDL_GetError());
            renderer = SDL_CreateRenderer(window, -1, 0);
        }
        if (renderer == nullptr) {
            ALOGE(MAC_SURFACE_TAG, "%s create renderer fail: %s", __func__, SDL_GetError());
            destroy();
            return NEGATIVE(S_NO_SDL_CREATE_RENDERER);
        }

        if (!SDL_GetRendererInfo(renderer, &rendererInfo)) {
            ALOGD(MAC_SURFACE_TAG, "initialized %s renderer", rendererInfo.name);
        }

        if (!window || !renderer || !rendererInfo.num_texture_formats) {
            ALOGD(MAC_SURFACE_TAG, "%s failed to create window or renderer: %s", __func__, SDL_GetError());
            destroy();
            return NEGATIVE(S_NO_SDL_INIT);
        }
    }
    return POSITIVE;
}

int MacSurface::destroy() {
    Surface::destroy();
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
    while (!quit && stream) {
        remainingTime = 0.0F;
        SDL_PumpEvents();
        while (!SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
            hideCursor();
            refreshVideo();
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
                if (stream && event.button.button == SDL_BUTTON_LEFT && !IS_NEGATIVE(isFullScreenClick())) {
                    toggleFullScreen();
                    stream->forceRefresh();
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
            if (options && stream) {
                options->surfaceWidth = stream->getVideoState()->width = event.window.data1;
                options->surfaceHeight = stream->getVideoState()->height = event.window.data2;
                if (videoTexture) {
                    SDL_DestroyTexture(videoTexture);
                    videoTexture = nullptr;
                }
            }
        case SDL_WINDOWEVENT_EXPOSED:
            if (stream) {
                stream->forceRefresh();
            }
        default:
            break;
    }
}

bool MacSurface::isQuitKey(const SDL_Event &event) const {
    return event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q;
}

bool MacSurface::isNotHaveWindow() const {
    // If we don't yet have a window, skip all lowres events, because read_thread might still be initializing...
    return !(stream && stream->getVideoState() && stream->getVideoState()->width);
}

void MacSurface::doKeySystem(const SDL_Event &event) const {
    switch (event.key.keysym.sym) {
        case SDLK_f:
            if (stream) {
                toggleFullScreen();
                stream->forceRefresh();
            }
            break;
        case SDLK_p:
        case SDLK_SPACE:
            if (stream) {
                stream->togglePause();
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
        case SDLK_s:
            if (stream) {
                stream->stepToNextFrame();
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

int MacSurface::displayWindow() {
    if (!Surface::displayWindow()) {
        return NEGATIVE(S_ERROR);
    }
    if (window && options && stream && stream->getVideoState()) {
        auto *macOptions = (MacOptions *) options;
        auto *videoState = stream->getVideoState();

        SDL_SetWindowTitle(window, options->videoTitle);
        SDL_SetWindowSize(window, videoState->width, videoState->height);
        SDL_SetWindowPosition(window, options->surfaceLeftOffset, options->surfaceTopOffset);
        if (macOptions->isFullScreen) {
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
        SDL_ShowWindow(window);
        ALOGD(MAC_SURFACE_TAG, "%s videoTitle = %s videoWidth = %d videoHeight = %d surfaceLeftOffset = %d surfaceTopOffset = %d", __func__,
              options->videoTitle,
              videoState->width, videoState->height,
              options->surfaceLeftOffset, options->surfaceTopOffset
        );
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int MacSurface::uploadVideoTexture(AVFrame *frame, SwsContext *convertContext) {
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

int MacSurface::uploadSubtitleTexture(Frame *nextSubtitleFrame, SwsContext *convertContext) {
    uint8_t *pixels[4];
    int pitch[4];
    int i;

    if (!reallocTexture(&subtitleTexture, SDL_PIXELFORMAT_ARGB8888, nextSubtitleFrame->width, nextSubtitleFrame->height, SDL_BLENDMODE_BLEND, 1)) {
        return NEGATIVE(S_NOT_REALLOC_TEXTURE);
    }

    for (i = 0; i < nextSubtitleFrame->sub.num_rects; i++) {
        AVSubtitleRect *subRect = nextSubtitleFrame->sub.rects[i];

        subRect->x = av_clip(subRect->x, 0, nextSubtitleFrame->width);
        subRect->y = av_clip(subRect->y, 0, nextSubtitleFrame->height);
        subRect->w = av_clip(subRect->w, 0, nextSubtitleFrame->width - subRect->x);
        subRect->h = av_clip(subRect->h, 0, nextSubtitleFrame->height - subRect->y);

        convertContext = sws_getCachedContext(convertContext, subRect->w, subRect->h, AV_PIX_FMT_PAL8, subRect->w, subRect->h, AV_PIX_FMT_BGRA, 0, NULL, NULL, NULL);
        if (convertContext) {
            ALOGE(MAC_SURFACE_TAG, "%s Cannot initialize the conversion context", __func__);
            return NEGATIVE(S_NOT_INIT_CONVERSION_CONTEXT);
        }

        SDL_Rect rect;
        rect.x = subRect->x;
        rect.y = subRect->y;
        rect.w = subRect->w;
        rect.h = subRect->h;
        if (!SDL_LockTexture(subtitleTexture, &rect, (void **) pixels, pitch)) {
            sws_scale(convertContext, (const uint8_t *const *) subRect->data, subRect->linesize, 0, subRect->h, pixels, pitch);
            SDL_UnlockTexture(subtitleTexture);
        }
    }
    return POSITIVE;
}

int MacSurface::updateSubtitleTexture(const AVSubtitleRect *subRect) const {
    uint8_t *pixels;
    int pitch, j;
    SDL_Rect rect;
    rect.x = subRect->x;
    rect.y = subRect->y;
    rect.w = subRect->w;
    rect.h = subRect->h;
    if (!SDL_LockTexture(subtitleTexture, &rect, (void **) &pixels, &pitch)) {
        for (j = 0; j < subRect->h; j++, pixels += pitch) {
            memset(pixels, 0, subRect->w << 2);
        }
        SDL_UnlockTexture(subtitleTexture);
    }
    return POSITIVE;
}

int MacSurface::displayVideoImageBefore() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    return POSITIVE;
}

int MacSurface::displayVideoImageAfter(Frame *currentFrame, Frame *nextSubtitleFrame, Rect *rect) {
    if (stream && stream->getVideoState()) {
        if (currentFrame) {
            SDL_Rect sdlRect;
            sdlRect.x = rect->x;
            sdlRect.y = rect->y;
            sdlRect.w = rect->w;
            sdlRect.h = rect->h;
            setYuvConversionMode(currentFrame->frame);
            SDL_RenderCopyEx(renderer, videoTexture, nullptr, &sdlRect, 0, nullptr, currentFrame->flipVertical ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE);
            setYuvConversionMode(nullptr);
        }
        if (nextSubtitleFrame) {
            int i;
            double xratio = (double) rect->w / (double) nextSubtitleFrame->width;
            double yratio = (double) rect->h / (double) nextSubtitleFrame->height;
            for (i = 0; i < nextSubtitleFrame->sub.num_rects; i++) {
                AVSubtitleRect *subRect = nextSubtitleFrame->sub.rects[i];

                SDL_Rect sdlSubRect;
                sdlSubRect.x = rect->x;
                sdlSubRect.y = rect->y;
                sdlSubRect.w = rect->w;
                sdlSubRect.h = rect->h;

                SDL_Rect sdlRect;
                sdlRect.x = (int) (rect->x + subRect->x * xratio);
                sdlRect.y = (int) (rect->y + subRect->y * yratio);
                sdlRect.w = (int) (subRect->w * xratio);
                sdlRect.h = (int) (subRect->h * yratio);

                SDL_RenderCopy(renderer, subtitleTexture, &sdlSubRect, &sdlRect);
            }
        }
        SDL_RenderPresent(renderer);
        return POSITIVE;
    }
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
}

void MacSurface::toggleFullScreen() const {
    if (options && window) {
        auto *macOptions = (MacOptions *) options;
        macOptions->isFullScreen = !macOptions->isFullScreen;
        SDL_SetWindowFullscreen(window, macOptions->isFullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
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
    if (options && stream && stream->getVideoState()) {
        VideoState *videoState = stream->getVideoState();
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
            stream->streamSeek((int64_t) pos, (int64_t) increment, 1);
        } else {
            pos = stream->getMasterClock();
            if (isnan(pos)) {
                pos = (double) videoState->seekPos / AV_TIME_BASE;
            }
            pos += increment;
            if (videoState->formatContext->start_time != AV_NOPTS_VALUE && pos < videoState->formatContext->start_time / (double) AV_TIME_BASE) {
                pos = videoState->formatContext->start_time / (double) AV_TIME_BASE;
            }
            stream->streamSeek((int64_t) (pos * AV_TIME_BASE), (int64_t) (increment * AV_TIME_BASE), 0);
        }
    }
}

AVPixelFormat *MacSurface::getPixelFormatsArray() {
    int nb_pix_fmts = 0;
    int i, j;
    for (i = 0; i < rendererInfo.num_texture_formats; i++) {
        for (j = 0; j < FF_ARRAY_ELEMS(SDL_TEXTURE_FORMAT_MAP) - 1; j++) {
            if (rendererInfo.texture_formats[i] == SDL_TEXTURE_FORMAT_MAP[j].textureFormat) {
                pix_fmts[nb_pix_fmts++] = SDL_TEXTURE_FORMAT_MAP[j].format;
                break;
            }
        }
    }
    pix_fmts[nb_pix_fmts] = AV_PIX_FMT_NONE;
    return pix_fmts;
}


