
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
    Rect *rect = new Rect();
    calculateDisplayRect(rect, 0, 0, width, height, width, height, rational);
    if (options) {
        options->screenWidth = rect->w;
        options->screenHeight = rect->h;
    }
    ALOGD(SURFACE_TAG, "%s x = %d y = %d w = %d h = %d", __func__, rect->x, rect->y, rect->w, rect->h);
    delete rect;
}

void Surface::displayWindow() {

}

void Surface::displayVideoImage() {
    if (play && play->getVideoState()) {
        displayVideoImageBefore();

        VideoState *videoState = play->getVideoState();
        Frame *currentToShowFrame = nullptr;
        Frame *sp = nullptr;
        Rect *rect = new Rect();
        currentToShowFrame = videoState->videoFrameQueue.peekCurrentToShowFrame();

        if (videoState->subtitleStream) {
            // TODO
        }

        calculateDisplayRect(rect, videoState->xLeft, videoState->yTop, videoState->width, videoState->height, currentToShowFrame->width, currentToShowFrame->height, currentToShowFrame->sampleAspectRatio);

        if (!currentToShowFrame->uploaded) {
            if (!uploadTexture(currentToShowFrame->frame, videoState->imgConvertCtx)) {
                return;
            }
            currentToShowFrame->uploaded = 1;
            currentToShowFrame->flipVertical = currentToShowFrame->frame->linesize[0] < 0;
        }
        displayVideoImageAfter(currentToShowFrame, rect);
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

void Surface::doExit() {
    if (play) {
        play->streamClose();
    }
    if (stream) {
        stream->destroy();
    }
    destroy();
}

void Surface::calculateDisplayRect(Rect *rect, int screenXLeft, int screenYTop, int screenWidth, int screenHeight, int pictureWidth, int pictureHeight, AVRational picSar) {
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

    ALOGD(SURFACE_TAG, "%s x = %d y = %d w = %d h = %d", __func__, rect->x, rect->y, rect->w, rect->h);
}

void Surface::displayVideoImageBefore() {

}

void Surface::displayVideoImageAfter(Frame *lastFrame, Rect *rect) {

}



