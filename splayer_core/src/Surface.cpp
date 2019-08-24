
#include <Surface.h>

#include "Surface.h"

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

void Surface::setWindowSize(int width, int height, AVRational rational) {
    if (options) {
        Rect *rect = new Rect();
        int max_width = options->screenWidth ? options->screenWidth : INT_MAX;
        int max_height = options->screenHeight ? options->screenHeight : INT_MAX;
        if (max_width == INT_MAX && max_height == INT_MAX) {
            max_height = height;
        }
        calculateDisplayRect(rect, 0, 0, max_width, max_height, width, height, rational);
        options->defaultWidth = rect->w;
        options->defaultHeight = rect->h;
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

void Surface::calculateDisplayRect(Rect *rect, int screenXLeft, int screenYTop, int screenWidth, int screenHeight, int pictureWidth, int pictureHeight, AVRational picSar) {

    AVRational aspect_ratio = picSar;
    int64_t width, height, x, y;

    if (av_cmp_q(aspect_ratio, av_make_q(0, 1)) <= 0)
        aspect_ratio = av_make_q(1, 1);

    aspect_ratio = av_mul_q(aspect_ratio, av_make_q(pictureWidth, pictureHeight));

    /* XXX: we suppose the screen has a 1.0 pixel ratio */
    height = screenHeight;
    width = av_rescale(height, aspect_ratio.num, aspect_ratio.den) & ~1;
    if (width > screenWidth) {
        width = screenWidth;
        height = av_rescale(width, aspect_ratio.den, aspect_ratio.num) & ~1;
    }
    x = (screenWidth - width) / 2;
    y = (screenHeight - height) / 2;
    rect->x = screenXLeft + x;
    rect->y = screenYTop + y;
    rect->w = FFMAX((int) width, 1);
    rect->h = FFMAX((int) height, 1);

    ALOGD(SURFACE_TAG, "%s x = %d y = %d w = %d h = %d", __func__, rect->x, rect->y, rect->w, rect->h);
}

int Surface::refreshVideo() {
    VideoState *videoState = stream->getVideoState();

    if (!videoState) {
        ALOGE(STREAM_TAG, "%s video status is null", __func__);
        return NEGATIVE(S_NULL);
    }

    ALOGD(STREAM_TAG, "===== refreshVideo =====");
    ALOGD(STREAM_TAG, "%s while remainingTime = %lf paused = %d forceRefresh = %d", __func__, remainingTime, videoState->paused, videoState->forceRefresh);
    if (remainingTime > 0.0) {
        av_usleep(static_cast<unsigned int>((int64_t) (remainingTime * AV_TIME_BASE)));
    }
    remainingTime = REFRESH_RATE;
    if (videoState->showMode != SHOW_MODE_NONE && (!videoState->paused || videoState->forceRefresh)) {
        _refreshVideo(&remainingTime);
    }
    ALOGD(STREAM_TAG, "===== end =====");

    return POSITIVE;
}

/* called to display each frame */
void Surface::_refreshVideo(double *remainingTime) {
    ALOGD(STREAM_TAG, "%s", __func__);
    double time;
    Frame *sp, *sp2;
    VideoState *videoState = stream->getVideoState();

    if (!videoState) {
        ALOGE(STREAM_TAG, "%s video status is null", __func__);
        return;
    }

    if (!videoState->paused && stream->getMasterSyncType() == SYNC_TYPE_EXTERNAL_CLOCK && videoState->realTime) {
        stream->checkExternalClockSpeed();
    }

    if (videoState->showMode != SHOW_MODE_VIDEO && videoState->audioStream) {
        time = av_gettime_relative() * 1.0F / AV_TIME_BASE;
        if (videoState->forceRefresh || (videoState->lastVisTime + options->rdftSpeed) < time) {
            displayVideo();
            videoState->lastVisTime = time;
        }
        *remainingTime = FFMIN(*remainingTime, videoState->lastVisTime + options->rdftSpeed - time);
    }

    if (videoState->videoStream) {
        retry:
        if (videoState->videoFrameQueue.numberRemaining() == 0) {
            ALOGD(STREAM_TAG, "nothing to do, no picture to display in the queue");
        } else {
            double lastDuration, duration, delay;
            Frame *willToShowFrame, *firstReadyToShowFrame;

            willToShowFrame = videoState->videoFrameQueue.peekWillToShowFrame();
            firstReadyToShowFrame = videoState->videoFrameQueue.peekFirstReadyToShowFrame();

            ALOGD(STREAM_TAG, "firstReadyToShowFrame serial = %d list serial = %d ", firstReadyToShowFrame->serial, videoState->videoPacketQueue.serial);
            if (firstReadyToShowFrame->serial != videoState->videoPacketQueue.serial) {
                videoState->videoFrameQueue.next();
                ALOGD(STREAM_TAG, "goto retry");
                goto retry;
            }

            ALOGD(STREAM_TAG, "willToShowFrame serial = %d firstReadyToShowFrame serial = %d ", willToShowFrame->serial, firstReadyToShowFrame->serial);
            if (willToShowFrame->serial != firstReadyToShowFrame->serial) {
                videoState->frameTimer = av_gettime_relative() * 1.0F / AV_TIME_BASE;
                ALOGD(STREAM_TAG, "update frameTimer = %fd ", videoState->frameTimer);
            }

            if (videoState->paused) {
                ALOGD(STREAM_TAG, "goto display");
                goto display;
            }

            /* compute nominal lastDuration */
            lastDuration = stream->getFrameDuration(willToShowFrame, firstReadyToShowFrame);
            delay = stream->getComputeTargetDelay(lastDuration);

            time = av_gettime_relative() * 1.0F / AV_TIME_BASE;
            double frameTimerDelay = videoState->frameTimer + delay;
            ALOGD(STREAM_TAG, "time = %f frameTimer = %f frameTimerDelay = %f", time, videoState->frameTimer, frameTimerDelay);
            if (time < frameTimerDelay) {
                *remainingTime = FFMIN(frameTimerDelay - time, *remainingTime);
                ALOGD(STREAM_TAG, "goto display diff time = %lf remainingTime = %lf", (frameTimerDelay - time), *remainingTime);
                goto display;
            }

            videoState->frameTimer += delay;
            ALOGD(STREAM_TAG, "frameTimer = %f ", videoState->frameTimer);
            if (delay > 0 && (time - videoState->frameTimer) > SYNC_THRESHOLD_MAX) {
                videoState->frameTimer = time;
                ALOGD(STREAM_TAG, "force set frameTimer = %f ", videoState->frameTimer);
            }

            videoState->videoFrameQueue.mutex->mutexLock();
            if (!isnan(firstReadyToShowFrame->pts)) {
                stream->updateVideoClockPts(firstReadyToShowFrame->pts, firstReadyToShowFrame->pos, firstReadyToShowFrame->serial);
            }
            videoState->videoFrameQueue.mutex->mutexUnLock();

            if (videoState->videoFrameQueue.numberRemaining() > 1) {
                Frame *nextPrepareShowFrame = videoState->videoFrameQueue.peekNextReadyToShowFrame();
                duration = stream->getFrameDuration(firstReadyToShowFrame, nextPrepareShowFrame);
                ALOGD(STREAM_TAG, "next frame duration = %lf ", duration);
                if (!videoState->step && (options->dropFrameWhenSlow > 0 || (options->dropFrameWhenSlow && stream->getMasterSyncType() != SYNC_TYPE_VIDEO_MASTER)) && time > (videoState->frameTimer + duration)) {
                    videoState->frameDropsLate++;
                    videoState->videoFrameQueue.next();
                    ALOGD(STREAM_TAG, "goto retry");
                    goto retry;
                }
            }

            // TODO subtitle

            videoState->videoFrameQueue.next();
            videoState->forceRefresh = 1;

            if (videoState->step && !videoState->paused) {
                stream->streamTogglePause();
            }
        }

        display:
        /* display picture */
        if (videoState->forceRefresh && videoState->showMode == SHOW_MODE_VIDEO && videoState->videoFrameQueue.readIndexShown) {
            displayVideo();
        }
    } else {
        ALOGD(STREAM_TAG, "not video streaming");
    }
    videoState->forceRefresh = 0;
}

/* display the current picture, if any */
void Surface::displayVideo() {
    ALOGD(STREAM_TAG, __func__);
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
            int width = options->screenWidth ? options->screenWidth : options->defaultWidth;
            int height = options->screenHeight ? options->screenHeight : options->defaultHeight;
            if (!options->windowTitle) {
                options->windowTitle = options->inputFileName;
            }
            videoState->width = width;
            videoState->height = height;
            return POSITIVE;
        }
    }
    return NEGATIVE(S_NULL);
}

void Surface::displayVideoAudio() {

}

void Surface::displayVideoImage() {
    if (stream && stream->getVideoState()) {
        displayVideoImageBefore();

        VideoState *videoState = stream->getVideoState();
        Frame *sp = nullptr;
        Rect *rect = new Rect();
        Frame *willToShowFrame = videoState->videoFrameQueue.peekWillToShowFrame();

        if (videoState->subtitleStream) {
            // TODO
        }

        calculateDisplayRect(rect, videoState->xLeft, videoState->yTop, videoState->width, videoState->height, willToShowFrame->width, willToShowFrame->height, willToShowFrame->sampleAspectRatio);

        if (!willToShowFrame->uploaded) {
            if (!uploadTexture(willToShowFrame->frame, videoState->imgConvertCtx)) {
                return;
            }
            willToShowFrame->uploaded = 1;
            willToShowFrame->flipVertical = willToShowFrame->frame->linesize[0] < 0;
        }

        displayVideoImageAfter(willToShowFrame, rect);
        delete rect;
    }
}

void Surface::displayVideoImageBefore() {

}

void Surface::displayVideoImageAfter(Frame *lastFrame, Rect *rect) {

}

AVPixelFormat *Surface::getPixelFormatsArray() {
    return nullptr;
}
