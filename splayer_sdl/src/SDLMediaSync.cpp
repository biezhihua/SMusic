#include <SDLVideoDevice.h>
#include "SDLMediaSync.h"

void SDLMediaSync::run() {
    bool quit = false;
    SDL_Event event;
    while (!quit) {
        resetRemainingTime();
        SDL_PumpEvents();
        while (!SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
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
//                if (stream && event.button.button == SDL_BUTTON_LEFT && !IS_NEGATIVE(isFullScreenClick())) {
//                    if (surface) {
//                        auto macSurface = (MacSurface *) surface;
//                        macSurface->toggleFullScreen();
//                    }
//                    stream->forceRefresh();
//                }
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
}


void SDLMediaSync::hideCursor() const {
//    if (options) {
//        auto *macOptions = (MacOptions * )(options);
//        if (!macOptions->cursorHidden && (av_gettime_relative() - macOptions->cursorLastShown) > CURSOR_HIDE_DELAY) {
//            SDL_ShowCursor(0);
//            macOptions->cursorHidden = 1;
//        }
//    }
}

void SDLMediaSync::showCursor() const {
//    if (options) {
//        auto *macOptions = (MacOptions * )(options);
//        if (macOptions->cursorHidden) {
//            SDL_ShowCursor(1);
//            macOptions->cursorHidden = 0;
//        }
//        macOptions->cursorLastShown = av_gettime_relative();
//    }
}

void SDLMediaSync::doWindowEvent(const SDL_Event &event) {
    switch (event.window.event) {
        case SDL_WINDOWEVENT_RESIZED:
            if (videoDevice) {
                auto *device = dynamic_cast<SDLVideoDevice *>(videoDevice);
                device->surfaceWidth = event.window.data1;
                device->surfaceHeight = event.window.data2;
                device->destroyVideoTexture();
            }
        case SDL_WINDOWEVENT_EXPOSED:
            forceRefresh = 1;
        default:
            break;
    }
}

int SDLMediaSync::isFullScreenClick() {
    int ret = -1;
    if ((av_gettime_relative() - lastMouseLeftClick) <= 5 * 100000) {
        lastMouseLeftClick = 0;
        ret = 0;
    } else {
        lastMouseLeftClick = av_gettime_relative();
    }
    return ret;
}


bool SDLMediaSync::isQuitKey(const SDL_Event &event) const {
    return event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q;
}

bool SDLMediaSync::isNotHaveWindow() const {
    // If we don't yet have a window, skip all lowres events, because read_thread might still be initializing...
    if (videoDevice) {
        auto *device = dynamic_cast<SDLVideoDevice *>(videoDevice);
        return !device->isDisplayWindow;
    }
    return true;
}

void SDLMediaSync::doKeySystem(const SDL_Event &event) const {
    switch (event.key.keysym.sym) {
        case SDLK_f:
//            if (stream) {
//                ((MacSurface *) surface)->toggleFullScreen();
//                stream->forceRefresh();
//            }
            break;
        case SDLK_p:
        case SDLK_SPACE:
//            if (stream) {
//                stream->togglePause();
//            }
            break;
        case SDLK_m:
            break;
        case SDLK_KP_MULTIPLY:
        case SDLK_0:
//            if (audio) {
//                audio->updateVolume(1, SDL_VOLUME_STEP);
//            }
            break;
        case SDLK_KP_DIVIDE:
        case SDLK_9:
//            if (audio) {
//                audio->updateVolume(-1, SDL_VOLUME_STEP);
//            }
            break;
        case SDLK_s:
//            if (stream) {
//                stream->stepToNextFrame();
//            }
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
//            if (options) {
//                doSeek(options->seekInterval != 0 ? -options->seekInterval : -10.0);
//            }
            break;
        case SDLK_RIGHT:
//            if (options) {
//                doSeek(options->seekInterval != 0 ? options->seekInterval : 10.0);
//            }
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


void SDLMediaSync::doExit() {
//    if (mediaPlayer) {
//        mediaPlayer->destroy();
//    }
}

void SDLMediaSync::doSeek(double increment) const {
//    double pos;
//    if (options && stream && stream->getVideoState()) {
//        PlayerState *videoState = stream->getVideoState();
//        if (options->seekByBytes) {
//            pos = -1;
//            if (pos < 0 && videoState->videoStreamIndex >= 0) {
//                pos = videoState->videoFrameQueue.lastPos();
//            }
//            if (pos < 0 && videoState->audioStreamIndex >= 0) {
//                pos = videoState->audioFrameQueue.lastPos();
//            }
//            if (pos < 0) {
//                pos = avio_tell(videoState->formatContext->pb);
//            }
//            if (videoState->formatContext->bit_rate) {
//                increment *= videoState->formatContext->bit_rate / 8.0;
//            } else {
//                increment *= 180000.0;
//            }
//            pos += increment;
//            stream->streamSeek((int64_t) pos, (int64_t) increment, 1);
//        } else {
//            pos = stream->getMasterClock();
//            if (isnan(pos)) {
//                pos = (double) videoState->seekPos / AV_TIME_BASE;
//            }
//            pos += increment;
//            if (videoState->formatContext->start_time != AV_NOPTS_VALUE &&
//                pos < videoState->formatContext->start_time / (double) AV_TIME_BASE) {
//                pos = videoState->formatContext->start_time / (double) AV_TIME_BASE;
//            }
//            stream->streamSeek((int64_t) (pos * AV_TIME_BASE), (int64_t) (increment * AV_TIME_BASE), 0);
//        }
//    }
}
