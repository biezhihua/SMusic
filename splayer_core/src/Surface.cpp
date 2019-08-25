
#include <Surface.h>

#include "Surface.h"
#include "../../../../../../usr/local/Cellar/sdl2/2.0.9_1/include/SDL2/SDL_render.h"

Surface::Surface() {
    mutex = new Mutex();
}

Surface::~Surface() {
    delete mutex;
}

int Surface::create() {
    return POSITIVE;
}

int Surface::destroy() {
    remainingTime = 0.0f;
    return POSITIVE;
}

void Surface::setStream(Stream *stream) {
    Surface::stream = stream;
}

void Surface::setVideoSize(int width, int height, AVRational rational) {
    if (options) {
        Rect *rect = new Rect();
        int maxWidth = options->surfaceWidth ? options->surfaceWidth : INT_MAX;
        int maxHeight = options->surfaceHeight ? options->surfaceHeight : INT_MAX;
        if (maxWidth == INT_MAX && maxHeight == INT_MAX) {
            maxHeight = height;
        }
        calculateDisplayRect(rect, 0, 0, maxWidth, maxHeight, width, height, rational);
        options->videoWidth = rect->w;
        options->videoHeight = rect->h;
        ALOGD(SURFACE_TAG, "%s x = %d y = %d w = %d h = %d", __func__, rect->x, rect->y, rect->w, rect->h);
        delete rect;
    }
}

void Surface::setOptions(Options *options) {
    Surface::options = options;
}

void Surface::setMediaPlayer(MediaPlayer *mediaPlayer) {
    Surface::mediaPlayer = mediaPlayer;
}

void Surface::calculateDisplayRect(Rect *rect, int xLeft, int yTop, int srcWidth, int scrHeight, int picWidth, int picHeight, AVRational picSar) {

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
    rect->x = xLeft + x;
    rect->y = yTop + y;
    rect->w = FFMAX((int) width, 1);
    rect->h = FFMAX((int) height, 1);
    ALOGD(SURFACE_TAG, "%s x = %d y = %d w = %d h = %d", __func__, rect->x, rect->y, rect->w, rect->h);
}

int Surface::refreshVideo() {
    VideoState *videoState = stream->getVideoState();

    if (!videoState) {
        ALOGE(SURFACE_TAG, "%s video status is null", __func__);
        return NEGATIVE(S_NULL);
    }

    ALOGD(SURFACE_TAG, "===== refreshVideo =====");
    ALOGD(SURFACE_TAG, "%s while remainingTime = %lf paused = %d forceRefresh = %d", __func__, remainingTime, videoState->paused, videoState->forceRefresh);
    if (remainingTime > 0.0) {
        av_usleep(static_cast<unsigned int>((int64_t) (remainingTime * AV_TIME_BASE)));
    }
    remainingTime = REFRESH_RATE;
    if (videoState->showMode != SHOW_MODE_NONE && (!videoState->paused || videoState->forceRefresh)) {
        refreshVideo(&remainingTime);
    }
    ALOGD(SURFACE_TAG, "===== end =====");

    return POSITIVE;
}

/* called to display each frame */
void Surface::refreshVideo(double *remainingTime) {
    double time;
    VideoState *videoState = stream->getVideoState();

    if (!videoState) {
        ALOGE(SURFACE_TAG, "%s video status is null", __func__);
        return;
    }

    if (!videoState->paused && stream->getMasterSyncType() == SYNC_TYPE_EXTERNAL_CLOCK && videoState->realTime) {
        stream->checkExternalClockSpeed();
    }

    if (!options->displayDisable && videoState->showMode != SHOW_MODE_VIDEO && videoState->audioStream) {
        time = av_gettime_relative() * 1.0F / AV_TIME_BASE;
        if (videoState->forceRefresh || (videoState->lastVisTime + options->rdftSpeed) < time) {
            displayVideo();
            videoState->lastVisTime = time;
        }
        *remainingTime = FFMIN(*remainingTime, videoState->lastVisTime + options->rdftSpeed - time);
    }

    if (videoState->videoStream) {

        ///RETRY
        retry:

        if (videoState->videoFrameQueue.numberRemaining() == 0) {
            ALOGD(SURFACE_TAG, "nothing to do, no picture to display in the queue");
        } else {
            double duration, delay;
            Frame *currentFrame;
            Frame *nextFrame;

            currentFrame = videoState->videoFrameQueue.peek();
            nextFrame = videoState->videoFrameQueue.peekNext();

            ALOGD(SURFACE_TAG, "nextFrame serial = %d packetQueue serial = %d ", nextFrame->serial, videoState->videoPacketQueue.serial);
            bool isNeedSyncSerial = nextFrame->serial != videoState->videoPacketQueue.serial;
            if (isNeedSyncSerial) {
                videoState->videoFrameQueue.next();
                ALOGE(SURFACE_TAG, "goto retry, need to sync serial");
                goto retry;
            }

            ALOGD(SURFACE_TAG, "currentFrame serial = %d nextFrame serial = %d ", currentFrame->serial, nextFrame->serial);
            if (currentFrame->serial != nextFrame->serial) {
                videoState->frameTimer = av_gettime_relative() * 1.0F / AV_TIME_BASE;
                ALOGD(SURFACE_TAG, "update frameTimer = %fd ", videoState->frameTimer);
            }

            if (videoState->paused) {
                ALOGD(SURFACE_TAG, "goto display, paused");
                goto display;
            }

            // 当前帧帧持续时间
            delay = getFrameDelayTime(currentFrame, nextFrame);

            // 当前帧时间
            time = av_gettime_relative() * 1.0F / AV_TIME_BASE;

            // 上一帧时间 + 当前帧持续时间
            double frameTimerDelay = videoState->frameTimer + delay;

            ALOGD(SURFACE_TAG, "time = %f frameTimer = %f frameTimerDelay = %f delay = %f", time, videoState->frameTimer, frameTimerDelay, delay);

            // 若上一帧持续显示时间超过当前帧的时间，那么代表上一帧还没显示结束，则继续显示当前帧
            bool isDelayCurrentFrame = time < frameTimerDelay;
            if (isDelayCurrentFrame) {
                // TODO: 计算不准确会影响CPU使用率
                *remainingTime = FFMIN(frameTimerDelay - time, *remainingTime);
                ALOGD(SURFACE_TAG, "goto display, need display pre frame, diff time = %lf remainingTime = %lf", (frameTimerDelay - time), *remainingTime);
                goto display;
            }

            // 累加当前帧持续时间
            videoState->frameTimer += delay;

            bool isNeedSyncFrameTime = delay > 0 && (time - videoState->frameTimer) > SYNC_THRESHOLD_MAX;
            if (isNeedSyncFrameTime) {
                videoState->frameTimer = time;
                ALOGD(SURFACE_TAG, "force set frameTimer = %f ", videoState->frameTimer);
            }

            videoState->videoFrameQueue.mutex->mutexLock();
            if (!isnan(nextFrame->pts)) {
                stream->updateVideoClockPts(nextFrame->pts, nextFrame->pos, nextFrame->serial);
            }
            videoState->videoFrameQueue.mutex->mutexUnLock();

            if (videoState->videoFrameQueue.numberRemaining() > 1) {

                Frame *nextPrepareShowFrame = videoState->videoFrameQueue.peekNextNext();
                duration = stream->getFrameDuration(nextFrame, nextPrepareShowFrame);
                ALOGD(SURFACE_TAG, "next frame duration = %lf ", duration);

                bool isNoStepFrame = !videoState->stepFrame;
                bool isDropFrameCondition = options->dropFrameWhenSlow > 0 || (options->dropFrameWhenSlow && stream->getMasterSyncType() != SYNC_TYPE_VIDEO_MASTER);
                bool isDropFrame = time > (videoState->frameTimer + duration);
                if (isNoStepFrame && isDropFrameCondition && isDropFrame) {
                    videoState->frameDropsLate++;
                    videoState->videoFrameQueue.next();
                    ALOGD(SURFACE_TAG, "goto retry, drop frame late");
                    goto retry;
                }
            }

            if (videoState->subtitleStream) {
                refreshSubtitle();
            }

            videoState->videoFrameQueue.next();
            videoState->forceRefresh = 1;

            if (videoState->stepFrame && !videoState->paused) {
                stream->streamTogglePause();
            }
        }

        /// DISPLAY
        display:
        if (videoState->forceRefresh && videoState->showMode == SHOW_MODE_VIDEO && videoState->videoFrameQueue.readIndexShown) {
            displayVideo();
        }
    } else {
        ALOGD(SURFACE_TAG, "not video streaming");
    }
    videoState->forceRefresh = 0;
}

void Surface::refreshSubtitle() const {
    if (!stream || !stream->getVideoState()) {
        return;
    }

    VideoState *videoState = stream->getVideoState();
    Frame *currentFrame;
    Frame *nextFrame;

    while (videoState->subtitleFrameQueue.numberRemaining() > 0) {
        currentFrame = videoState->subtitleFrameQueue.peek();
        if (videoState->subtitleFrameQueue.numberRemaining() > 1) {
            nextFrame = videoState->subtitleFrameQueue.peekNext();
        } else {
            nextFrame = nullptr;
        }

        bool isNoSamePacketSerial = currentFrame->serial != videoState->subtitlePacketQueue.serial;
        bool isNeedDropFrame = videoState->videoClock.pts > (currentFrame->pts + ((float) currentFrame->sub.end_display_time / 1000));
        bool isNeedDropNextFrame = nextFrame != nullptr && videoState->videoClock.pts > (nextFrame->pts + ((float) nextFrame->sub.start_display_time / 1000));
        if (isNoSamePacketSerial || isNeedDropFrame || isNeedDropNextFrame) {
            if (currentFrame->uploaded) {
                for (int i = 0; i < currentFrame->sub.num_rects; i++) {
                    AVSubtitleRect *subtitleRect = currentFrame->sub.rects[i];
                    updateSubtitleTexture(subtitleRect);
                }
            }
            videoState->subtitleFrameQueue.next();
        } else {
            break;
        }
    }
}

int Surface::updateSubtitleTexture(const AVSubtitleRect *sub_rect) const {
    return POSITIVE;
}

double Surface::getFrameDelayTime(const Frame *willToShowFrame, const Frame *firstReadyToShowFrame) const {
    double duration = stream->getFrameDuration(willToShowFrame, firstReadyToShowFrame);
    double delay = stream->getComputeTargetDelay(duration);
    return delay;
}

/* display the current picture, if any */
void Surface::displayVideo() {
    ALOGD(SURFACE_TAG, __func__);
    VideoState *videoState = stream->getVideoState();
    if (videoState) {
        if (!videoState->width) {
            displayWindow();
        }
        if (videoState->audioStream && videoState->showMode != SHOW_MODE_VIDEO) {
            displayVideoAudio();
        } else if (videoState->videoStream) {
            displayVideoImage();
        }
    }
}

int Surface::displayWindow() {
    if (options) {
        VideoState *videoState = stream->getVideoState();
        if (videoState) {
            int width = options->surfaceWidth ? options->surfaceWidth : options->videoWidth;
            int height = options->surfaceHeight ? options->surfaceHeight : options->videoHeight;
            if (!options->videoTitle) {
                options->videoTitle = options->inputFileName;
            }
            videoState->width = width;
            videoState->height = height;
            return POSITIVE;
        }
    }
    return NEGATIVE(S_NULL);
}

int Surface::displayVideoAudio() {
    return POSITIVE;
}

void Surface::displayVideoImage() {
    if (stream && stream->getVideoState()) {
        displayVideoImageBefore();

        VideoState *videoState = stream->getVideoState();
        Rect *rect = new Rect();
        Frame *currentFrame = videoState->videoFrameQueue.peek();
        Frame *nextSubtitleFrame = nullptr;

        if (videoState->subtitleStream) {
            displaySubtitleImage(videoState, currentFrame, nextSubtitleFrame);
        }

        calculateDisplayRect(rect, videoState->xLeft, videoState->yTop, videoState->width, videoState->height, currentFrame->width, currentFrame->height, currentFrame->sampleAspectRatio);

        if (!currentFrame->uploaded) {
            if (!uploadVideoTexture(currentFrame->frame, videoState->imgConvertCtx)) {
                return;
            }
            currentFrame->uploaded = 1;
            currentFrame->flipVertical = currentFrame->frame->linesize[0] < 0;
        }

        displayVideoImageAfter(currentFrame, nextSubtitleFrame, rect);
        delete rect;
    }
}

void Surface::displaySubtitleImage(VideoState *videoState, const Frame *currentFrame, Frame *nextSubtitleFrame) {
    if (videoState->subtitleFrameQueue.numberRemaining() > 0) {
        nextSubtitleFrame = videoState->subtitleFrameQueue.peekNext();
        if (currentFrame->pts >= nextSubtitleFrame->pts + ((float) nextSubtitleFrame->sub.start_display_time / 1000)) {
            if (!nextSubtitleFrame->uploaded) {
                if (!nextSubtitleFrame->width || !nextSubtitleFrame->height) {
                    nextSubtitleFrame->width = currentFrame->width;
                    nextSubtitleFrame->height = currentFrame->height;
                }
                if (!uploadSubtitleTexture(nextSubtitleFrame, videoState->subConvertCtx)) {
                    return;
                }
                nextSubtitleFrame->uploaded = 1;
            }
        } else {
            nextSubtitleFrame = nullptr;
        }
    }
}

int Surface::displayVideoImageBefore() {
    return 0;
}


AVPixelFormat *Surface::getPixelFormatsArray() {
    return nullptr;
}

void Surface::setMsgQueue(MessageQueue *msgQueue) {
    Surface::msgQueue = msgQueue;
}

int Surface::displayVideoImageAfter(Frame *currentFrame, Frame *subtitleNextFrame, Rect *rect) {
    return 0;
}

int Surface::uploadSubtitleTexture(Frame *nextFrame, SwsContext *convertCtx) {
    return 0;
}
