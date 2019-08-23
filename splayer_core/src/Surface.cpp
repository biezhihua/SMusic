
#include <Surface.h>

#include "Surface.h"

Surface::Surface() {
    mutex = new Mutex();
}

Surface::~Surface() {
    delete mutex;
}

void Surface::setPlay(FFPlay *play) {
    Surface::play = play;
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

void Surface::displayWindow(int width, int height) {

}

void Surface::displayVideoImage() {
    if (play && play->getVideoState()) {
        displayVideoImageBefore();

        VideoState *videoState = play->getVideoState();
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

Options *Surface::getOptions() const {
    return options;
}

void Surface::setOptions(Options *options) {
    Surface::options = options;
}

Stream *Surface::getStream() const {
    return stream;
}

void Surface::setStream(Stream *stream) {
    Surface::stream = stream;
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

void Surface::displayVideoImageBefore() {

}

void Surface::displayVideoImageAfter(Frame *lastFrame, Rect *rect) {

}

MediaPlayer *Surface::getMediaPlayer() const {
    return mediaPlayer;
}

void Surface::setMediaPlayer(MediaPlayer *mediaPlayer) {
    Surface::mediaPlayer = mediaPlayer;
}



