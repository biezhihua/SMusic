#include "MacEvent.h"

int MacEvent::eventLoop() {
    bool quit = false;
    SDL_Event event;
    auto *macSurface = (MacSurface *) surface;
    auto *macStream = (MacStream *) stream;
    while (!quit && macStream && macSurface) {
        macSurface->resetRemainingTime();
        SDL_PumpEvents();
        while (!SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
            hideCursor();
            macSurface->refreshVideo();
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
                    macSurface->toggleFullScreen();
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

void MacEvent::hideCursor() const {
    if (options) {
        auto *macOptions = (MacOptions *) (options);
        if (!macOptions->cursorHidden && (av_gettime_relative() - macOptions->cursorLastShown) > CURSOR_HIDE_DELAY) {
            SDL_ShowCursor(0);
            macOptions->cursorHidden = 1;
        }
    }
}

void MacEvent::showCursor() const {
    if (options) {
        auto *macOptions = (MacOptions *) (options);
        if (macOptions->cursorHidden) {
            SDL_ShowCursor(1);
            macOptions->cursorHidden = 0;
        }
        macOptions->cursorLastShown = av_gettime_relative();
    }
}

void MacEvent::doWindowEvent(const SDL_Event &event) {
    ALOGD(MAC_EVENT_TAG, "%s width = %d height = %d ", __func__, event.window.data1, event.window.data2);
    if (surface) {
        auto *macSurface = (MacSurface *) surface;
        switch (event.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
                if (options && stream) {
                    options->surfaceWidth = stream->getVideoState()->width = event.window.data1;
                    options->surfaceHeight = stream->getVideoState()->height = event.window.data2;
                    macSurface->destroyVideoTexture();
                }
            case SDL_WINDOWEVENT_EXPOSED:
                if (stream) {
                    stream->forceRefresh();
                }
            default:
                break;
        }
    }
}

int MacEvent::isFullScreenClick() {
    int ret = NEGATIVE(S_ERROR);
    if ((av_gettime_relative() - lastMouseLeftClick) <= 5 * 100000) {
        lastMouseLeftClick = 0;
        ret = POSITIVE;
    } else {
        lastMouseLeftClick = av_gettime_relative();
    }
    return ret;
}


bool MacEvent::isQuitKey(const SDL_Event &event) const {
    return event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q;
}

bool MacEvent::isNotHaveWindow() const {
    // If we don't yet have a window, skip all lowres events, because read_thread might still be initializing...
    return !(stream && stream->getVideoState() && stream->getVideoState()->width);
}

void MacEvent::doKeySystem(const SDL_Event &event) const {
    switch (event.key.keysym.sym) {
        case SDLK_f:
            if (stream) {
                ((MacSurface *) surface)->toggleFullScreen();
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


void MacEvent::doExit() {
    if (mediaPlayer) {
        mediaPlayer->destroy();
    }
}

void MacEvent::doSeek(double increment) const {
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