#include <SDLVideoDevice.h>

SDLVideoDevice::SDLVideoDevice() : VideoDevice() {
}

SDLVideoDevice::~SDLVideoDevice() = default;

int SDLVideoDevice::create() {
    unsigned int initFlags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;

    if (!SDL_getenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE")) {
        SDL_setenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE", "1", 1);
    }

    if (SDL_Init(initFlags) != 0) {
        if (DEBUG) {
            ALOGD(TAG, "[%s] init sdl fail code = %s", __func__, SDL_GetError());
        }
        destroy();
        return ERROR;
    }

    Uint32 windowFlags = SDL_WINDOW_HIDDEN;

    windowFlags |= SDL_WINDOW_RESIZABLE;

    window = SDL_CreateWindow("SPlayer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, surfaceWidth, surfaceHeight,
                              windowFlags);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

    if (!window) {
        if (DEBUG) {
            ALOGE(TAG, "[%s] create sdl window fail: %s", __func__, SDL_GetError());
        }
        destroy();
        return ERROR_NOT_MEMORY;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        if (DEBUG) {
            ALOGE(TAG, "[%s] failed to initialize a hardware accelerated renderer: %s", __func__, SDL_GetError());
        }
        renderer = SDL_CreateRenderer(window, -1, 0);
    }
    if (!renderer) {
        if (DEBUG) {
            ALOGE(TAG, "[%s] create renderer fail: %s", __func__, SDL_GetError());
        }
        destroy();
        return ERROR_NOT_MEMORY;
    }

    if (!SDL_GetRendererInfo(renderer, &rendererInfo)) {
        if (DEBUG) {
            ALOGD(TAG, "[%s] initialized %s renderer", __func__, rendererInfo.name);
        }
    }
    if (!window || !renderer || !rendererInfo.num_texture_formats) {
        if (DEBUG) {
            ALOGD(TAG, "[%s] failed to create window or renderer: %s", __func__, SDL_GetError());
        }
        destroy();
        return ERROR_NOT_MEMORY;
    }
    return SUCCESS;
}

int SDLVideoDevice::destroy() {
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
    return SUCCESS;
}

int
SDLVideoDevice::onInitTexture(int initTexture, int newWidth, int newHeight, TextureFormat newFormat,
                              BlendMode blendMode, int rotate) {
    Uint32 format;
    int access;
    int width;
    int height;
    bool cond1 = videoTexture != nullptr;
    bool cond2 = SDL_QueryTexture(videoTexture, &format, &access, &width, &height) < 0;
    bool cond3 = newWidth != width;
    bool cond4 = newHeight != height;
    bool cond5 = newFormat != getTextureFormat(format);
    if (cond1 || cond2 || cond3 || cond4 || cond5) {
        void *pixels;
        int pitch;
        if (videoTexture) {
            SDL_DestroyTexture(videoTexture);
        }
        if (!(videoTexture = SDL_CreateTexture(renderer, getSDLFormat(newFormat), SDL_TEXTUREACCESS_STREAMING, newWidth,
                                               newHeight))) {
            return ERROR;
        }
        if (SDL_SetTextureBlendMode(videoTexture, getSDLBlendMode(blendMode)) < 0) {
            return ERROR;
        }
        if (initTexture) {
            if (SDL_LockTexture(videoTexture, nullptr, &pixels, &pitch) < 0) {
                return ERROR;
            }
            memset(pixels, 0, pitch * newHeight);
            SDL_UnlockTexture(videoTexture);
        }
        if (DEBUG) {
            ALOGD(TAG, "[%s] Created %dx%d texture with %s", __func__, newWidth, newHeight,
                  SDL_GetPixelFormatName(newFormat));
        }
    }
    return SUCCESS;
}

int SDLVideoDevice::onUpdateYUV(uint8_t *yData, int yPitch, uint8_t *uData, int uPitch, uint8_t *vData, int vPitch) {
    return SDL_UpdateYUVTexture(videoTexture, nullptr, yData, yPitch, uData, uPitch, vData, vPitch);
}

int SDLVideoDevice::onUpdateARGB(uint8_t *rgba, int pitch) {
    return VideoDevice::onUpdateARGB(rgba, pitch);
}

void SDLVideoDevice::onRequestRenderStart(Frame *frame) {
    if (!isDisplayWindow) {
        setSurfaceSize(frame);
        displayWindow();
        isDisplayWindow = true;
    }
    if (renderer != nullptr) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
    }
}

int SDLVideoDevice::onRequestRenderEnd(Frame *frame, bool flip) {
    if (renderer != nullptr) {
        if (frame) {
            SDL_Rect rect;
            calculateDisplayRect(&rect, surfaceLeftOffset, surfaceTopOffset, surfaceWidth, surfaceHeight, frame->width,
                                 frame->height, frame->sampleAspectRatio);
            setYuvConversionMode(frame->frame);
            if (DEBUG) {
                ALOGD(TAG, "[%s] x = %d y = %d w = %d h = %d", __func__, rect.x, rect.y, rect.w, rect.h);
            }
            SDL_RenderCopyEx(renderer, videoTexture, nullptr, &rect, 0, nullptr,
                             flip ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE);
            setYuvConversionMode(nullptr);
        }
        SDL_RenderPresent(renderer);
        return SUCCESS;
    }
    return ERROR;
}

void SDLVideoDevice::calculateDisplayRect(SDL_Rect *rect,
                                          int xLeft, int yTop,
                                          int srcWidth, int scrHeight,
                                          int picWidth, int picHeight,
                                          AVRational picSar) {

    if (DEBUG) {
        ALOGD(TAG, "[%s] srcWidth = %d scrHeight = %d picWidth = %d picHeight = %d", __func__, srcWidth, scrHeight,
              picWidth,
              picHeight);
    }
    AVRational aspectRatio = picSar;
    int64_t width, height, x, y;

    if (av_cmp_q(aspectRatio, av_make_q(0, 1)) <= 0) {
        aspectRatio = av_make_q(1, 1);
    }

    aspectRatio = av_mul_q(aspectRatio, av_make_q(picWidth, picHeight));

    /* XXX: we suppose the screen has a 1.0 pixel ratio */
    height = scrHeight;
    width = av_rescale(height, aspectRatio.num, aspectRatio.den) & ~1;
    if (width > srcWidth) {
        width = srcWidth;
        height = av_rescale(width, aspectRatio.den, aspectRatio.num) & ~1;
    }
    x = (srcWidth - width) / 2;
    y = (scrHeight - height) / 2;
    rect->x = static_cast<int>(xLeft + x);
    rect->y = static_cast<int>(yTop + y);
    rect->w = FFMAX((int) width, 1);
    rect->h = FFMAX((int) height, 1);
    if (DEBUG) {
        ALOGD(TAG, "[%s] x = %d y = %d w = %d h = %d", __func__, rect->x, rect->y, rect->w, rect->h);
    }
}


void SDLVideoDevice::setYuvConversionMode(AVFrame *frame) {
#if SDL_VERSION_ATLEAST(2, 0, 8)
    SDL_YUV_CONVERSION_MODE mode = SDL_YUV_CONVERSION_AUTOMATIC;
    if (frame && (frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUYV422 ||
                  frame->format == AV_PIX_FMT_UYVY422)) {
        if (frame->color_range == AVCOL_RANGE_JPEG) {
            mode = SDL_YUV_CONVERSION_JPEG;
            if (DEBUG) {
                ALOGD(TAG, "[%s] mode = %s", __func__, "SDL_YUV_CONVERSION_JPEG");
            }
        } else if (frame->colorspace == AVCOL_SPC_BT709) {
            mode = SDL_YUV_CONVERSION_BT709;
            if (DEBUG) {
                ALOGD(TAG, "[%s] mode = %s", __func__, "SDL_YUV_CONVERSION_BT709");
            }
        } else if (frame->colorspace == AVCOL_SPC_BT470BG || frame->colorspace == AVCOL_SPC_SMPTE170M ||
                   frame->colorspace == AVCOL_SPC_SMPTE240M) {
            mode = SDL_YUV_CONVERSION_BT601;
            if (DEBUG) {
                ALOGD(TAG, "[%s] mode = [%s]", __func__, "SDL_YUV_CONVERSION_BT601");
            }
        }
    }
    SDL_SetYUVConversionMode(mode);
#endif
}

SDL_BlendMode SDLVideoDevice::getSDLBlendMode(BlendMode mode) {
    switch (mode) {
        case BLEND_NONE:
            return SDL_BLENDMODE_NONE;
        case BLEND_BLEND:
            return SDL_BLENDMODE_BLEND;
        case BLEND_ADD:
            return SDL_BLENDMODE_ADD;
        case BLEND_MOD:
            return SDL_BLENDMODE_MOD;
        case BLEND_INVALID:
            return SDL_BLENDMODE_INVALID;
    }
    return SDL_BLENDMODE_NONE;
}

TextureFormat SDLVideoDevice::getTextureFormat(Uint32 format) {
    switch (format) {
        case SDL_PIXELFORMAT_RGB332:
            return FMT_RGB8;
        case SDL_PIXELFORMAT_RGB444:
            return FMT_RGB444;
        case SDL_PIXELFORMAT_RGB555:
            return FMT_RGB555;
        case SDL_PIXELFORMAT_BGR555:
            return FMT_BGR555;
        case SDL_PIXELFORMAT_RGB565:
            return FMT_RGB565;
        case SDL_PIXELFORMAT_BGR565:
            return FMT_BGR565;
        case SDL_PIXELFORMAT_RGB24:
            return FMT_RGB24;
        case SDL_PIXELFORMAT_BGR24:
            return FMT_BGR24;
        case SDL_PIXELFORMAT_RGB888:
            return FMT_0RGB32;
        case SDL_PIXELFORMAT_BGR888:
            return FMT_0BGR32;
        case SDL_PIXELFORMAT_RGBX8888:
            return FMT_NE_RGBX;
        case SDL_PIXELFORMAT_BGRX8888:
            return FMT_NE_BGRX;
        case SDL_PIXELFORMAT_ARGB8888:
            return FMT_RGB32;
        case SDL_PIXELFORMAT_RGBA8888:
            return FMT_RGB32_1;
        case SDL_PIXELFORMAT_ABGR8888:
            return FMT_BGR32;
        case SDL_PIXELFORMAT_BGRA8888:
            return FMT_BGR32_1;
        case SDL_PIXELFORMAT_IYUV:
            return FMT_YUV420P;
        case SDL_PIXELFORMAT_YUY2:
            return FMT_YUYV422;
        case SDL_PIXELFORMAT_UYVY:
            return FMT_UYVY422;
        case SDL_PIXELFORMAT_UNKNOWN:
            return FMT_NONE;
        default:
            return FMT_NONE;
    }
}

void SDLVideoDevice::displayWindow() {

    if (!playerInfoStatus->videoTitle) {
        playerInfoStatus->videoTitle = playerInfoStatus->url;
    }

    int width = surfaceWidth ? surfaceWidth : defaultWidth;
    int height = surfaceHeight ? surfaceHeight : defaultHeight;

    if (window) {
        SDL_SetWindowTitle(window, playerInfoStatus->videoTitle);
        SDL_SetWindowSize(window, width, height);
        SDL_SetWindowPosition(window, surfaceLeftOffset, surfaceTopOffset);
        SDL_ShowWindow(window);
        if (DEBUG) {
            ALOGD(TAG,
                  "[%s] videoTitle = %s videoWidth = %d videoHeight = %d surfaceLeftOffset = %d surfaceTopOffset = %d",
                  __func__,
                  playerInfoStatus->videoTitle,
                  width, height,
                  surfaceLeftOffset, surfaceTopOffset
            );
        }
    }
}

void SDLVideoDevice::setSurfaceSize(Frame *frame) {
    int width = frame->width;
    int height = frame->height;
    AVRational rational = frame->sampleAspectRatio;
    SDL_Rect rect;
    int maxWidth = surfaceWidth ? surfaceWidth : INT_MAX;
    int maxHeight = surfaceHeight ? surfaceHeight : INT_MAX;
    if (maxWidth == INT_MAX && maxHeight == INT_MAX) {
        maxHeight = height;
    }
    calculateDisplayRect(&rect, 0, 0, maxWidth, maxHeight, width, height, rational);
    surfaceWidth = rect.w;
    surfaceHeight = rect.h;

    if (DEBUG) {
        ALOGD(TAG, "[%s] surfaceWidth = %d surfaceHeight = %d", __func__, surfaceWidth, surfaceHeight);
    }
}

Uint32 SDLVideoDevice::getSDLFormat(TextureFormat format) {
    switch (format) {
        case FMT_RGB8:
            return SDL_PIXELFORMAT_RGB332;
        case FMT_RGB444:
            return SDL_PIXELFORMAT_RGB444;
        case FMT_RGB555:
            return SDL_PIXELFORMAT_RGB555;
        case FMT_BGR555:
            return SDL_PIXELFORMAT_BGR555;
        case FMT_RGB565:
            return SDL_PIXELFORMAT_RGB565;
        case FMT_BGR565:
            return SDL_PIXELFORMAT_BGR565;
        case FMT_RGB24:
            return SDL_PIXELFORMAT_RGB24;
        case FMT_BGR24:
            return SDL_PIXELFORMAT_BGR24;
        case FMT_0RGB32:
            return SDL_PIXELFORMAT_RGB888;
        case FMT_0BGR32:
            return SDL_PIXELFORMAT_BGR888;
        case FMT_NE_RGBX:
            return SDL_PIXELFORMAT_RGBX8888;
        case FMT_NE_BGRX:
            return SDL_PIXELFORMAT_BGRX8888;
        case FMT_RGB32:
            return SDL_PIXELFORMAT_ARGB8888;
        case FMT_RGB32_1:
            return SDL_PIXELFORMAT_RGBA8888;
        case FMT_BGR32:
            return SDL_PIXELFORMAT_ABGR8888;
        case FMT_BGR32_1:
            return SDL_PIXELFORMAT_BGRA8888;
        case FMT_YUV420P:
            return SDL_PIXELFORMAT_IYUV;
        case FMT_YUYV422:
            return SDL_PIXELFORMAT_YUY2;
        case FMT_UYVY422:
            return SDL_PIXELFORMAT_UYVY;
        case FMT_NONE:
            return SDL_PIXELFORMAT_UNKNOWN;
        default:
            return SDL_PIXELFORMAT_UNKNOWN;
    }
}

void SDLVideoDevice::destroyVideoTexture() {
    if (videoTexture) {
        SDL_DestroyTexture(videoTexture);
        videoTexture = nullptr;
    }
}

void SDLVideoDevice::toggleFullScreen() {
    if (window) {
        isFullScreen = !isFullScreen;
        SDL_SetWindowFullscreen(window, isFullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    }
}



