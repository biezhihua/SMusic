#ifndef SPLAYER_VOUT_H
#define SPLAYER_VOUT_H

class Stream;

class MediaPlayer;

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "Log.h"
#include "Stream.h"
#include "Mutex.h"
#include "Options.h"
#include "Rect.h"
#include "Frame.h"
#include "MediaPlayer.h"
#include "VideoState.h"

extern "C" {
#include <libavutil/rational.h>
}

#define SURFACE_TAG "Surface"

class Surface {

protected:
    MediaPlayer *mediaPlayer = nullptr;
    Stream *stream = nullptr;
    Mutex *mutex = nullptr;
    Options *options = nullptr;
    MessageQueue *msgQueue = nullptr;
    double remainingTime = 0.0f;

public:
    Surface();

    virtual ~Surface();

    virtual int create();

    virtual int destroy();

    void setVideoSize(int width, int height, AVRational rational);

    void setStream(Stream *stream);

    void setOptions(Options *options);

    void setMediaPlayer(MediaPlayer *mediaPlayer);

    void setMsgQueue(MessageQueue *msgQueue);

    virtual AVPixelFormat *getPixelFormatsArray();

private:

    void refreshVideo(double *remainingTime);

    void refreshSubtitle() const;

    double getFrameDelayTime(const Frame *willToShowFrame, const Frame *firstReadyToShowFrame) const;

    void calculateDisplayRect(Rect *rect, int xLeft, int yTop, int srcWidth, int scrHeight, int picWidth, int picHeight, AVRational picSar);

    void displayVideo();

    void displayVideoImage();

    void displaySubtitleImage(VideoState *videoState, const Frame *currentFrame, Frame *nextSubtitleFrame);

protected:

    int refreshVideo();

    virtual int displayWindow();

    virtual int displayVideoImageBefore();

    virtual int displayVideoImageAfter(Frame *currentFrame, Frame *subtitleNextFrame, Rect *rect);

    virtual int uploadVideoTexture(AVFrame *frame, SwsContext *convertContext) = 0;

    virtual int displayVideoAudio();

    virtual int updateSubtitleTexture(const AVSubtitleRect *sub_rect) const;

    virtual int uploadSubtitleTexture(Frame *nextFrame, SwsContext *convertCtx);
};


#endif //SPLAYER_VOUT_H
