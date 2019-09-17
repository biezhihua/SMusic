#include <SDLMediaPlayer.h>

int SDLMediaPlayer::eventLoop() {
    SDL_Event event;
    while (!quit) {
        resetRemainingTime();
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
                    break;
                }
                if (isNotHaveWindow()) {
                    continue;
                }
                doKeySystem(event);
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT && isFullScreenClick() >= 0) {
                    toggleFullScreen();
                }
            case SDL_MOUSEMOTION:
                showCursor();
                break;
            case SDL_WINDOWEVENT:
                doWindowEvent(event);
                break;
            case SDL_QUIT:
                doExit();
                break;
            default:
                break;
        }
    }
}

void SDLMediaPlayer::toggleFullScreen() {
    mutex.lock();
    if (videoDevice) {
        auto *device = (SDLVideoDevice *) (videoDevice);
        device->toggleFullScreen();
    }
    if (mediaSync) {
        mediaSync->setForceRefresh(1);
    }
    mutex.unlock();
}

void SDLMediaPlayer::refreshVideo() {
    mutex.lock();
    if (mediaSync != nullptr) {
        mediaSync->refreshVideo();
    }
    mutex.unlock();
}

void SDLMediaPlayer::resetRemainingTime() {
    mutex.lock();
    if (mediaSync != nullptr) {
        mediaSync->resetRemainingTime();
    }
    mutex.unlock();
}


void SDLMediaPlayer::hideCursor() {
    mutex.lock();
    if (videoDevice != nullptr) {
        auto *device = (SDLVideoDevice *) (videoDevice);
        if (!device->cursorHidden && (av_gettime_relative() - device->cursorLastShown) > CURSOR_HIDE_DELAY) {
            SDL_ShowCursor(0);
            device->cursorHidden = 1;
        }
    }
    mutex.unlock();
}

void SDLMediaPlayer::showCursor() {
    mutex.lock();
    if (videoDevice != nullptr) {
        auto *device = (SDLVideoDevice *) (videoDevice);
        if (device->cursorHidden) {
            SDL_ShowCursor(1);
            device->cursorHidden = 0;
        }
        device->cursorLastShown = av_gettime_relative();
    }
    mutex.unlock();
}

void SDLMediaPlayer::doWindowEvent(const SDL_Event &event) {
    switch (event.window.event) {
        case SDL_WINDOWEVENT_RESIZED:
            if (videoDevice) {
                auto *device = dynamic_cast<SDLVideoDevice *>(videoDevice);
                device->surfaceWidth = event.window.data1;
                device->surfaceHeight = event.window.data2;
                device->destroyVideoTexture();
            }
        case SDL_WINDOWEVENT_EXPOSED:
            if (mediaSync) {
                mediaSync->setForceRefresh(1);
            }
        default:
            break;
    }
}

int SDLMediaPlayer::isFullScreenClick() {
    int ret = -1;
    if ((av_gettime_relative() - lastMouseLeftClick) <= 5 * 100000) {
        lastMouseLeftClick = 0;
        ret = 0;
    } else {
        lastMouseLeftClick = av_gettime_relative();
    }
    return ret;
}


bool SDLMediaPlayer::isQuitKey(const SDL_Event &event) {
    return event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q;
}

bool SDLMediaPlayer::isNotHaveWindow() {
    // If we don't yet have a window, skip all lowres events, because read_thread might still be initializing...
    mutex.lock();
    bool ret = false;
    if (videoDevice) {
        auto *device = dynamic_cast<SDLVideoDevice *>(videoDevice);
        ret = !device->isDisplayWindow;
    }
    mutex.unlock();
    return ret;
}

void SDLMediaPlayer::doKeySystem(const SDL_Event &event) {
    switch (event.key.keysym.sym) {
        case SDLK_f:
            if (videoDevice && mediaSync) {
                auto *device = (SDLVideoDevice *) (videoDevice);
                device->toggleFullScreen();
                mediaSync->setForceRefresh(1);
            }
            break;
        case SDLK_o:
            break;
        case SDLK_p:
            notifyMsg(Msg::MSG_REQUEST_START);
            break;
        case SDLK_SPACE:
            notifyMsg(Msg::MSG_REQUEST_PLAY_OR_PAUSE);
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


void SDLMediaPlayer::doExit() {
    notifyMsg(Msg::MSG_REQUEST_DESTROY);
}

void SDLMediaPlayer::doSeek(double increment) {
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

int SDLMediaPlayer::destroy() {
    ALOGD(TAG, "destroy sdl media player - start");
    mutex.lock();
    MediaPlayer::_destroy();
    quit = true;
    mutex.unlock();
    ALOGD(TAG, "destroy sdl media player - end");
    return SUCCESS;
}

