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

    double remainingTime = 0.0f;

public:
    Surface();

    virtual ~Surface();

    virtual int create();

    virtual int destroy();

    void setWindowSize(int width, int height, AVRational rational);

    void setStream(Stream *stream);

    void setOptions(Options *options);

    void setMediaPlayer(MediaPlayer *mediaPlayer);

    virtual AVPixelFormat *getPixelFormatsArray();

protected:

    void calculateDisplayRect(Rect *rect, int screenXLeft, int screenYTop, int screenWidth, int screenHeight, int pictureWidth, int pictureHeight, AVRational picSar);

    int refreshVideo();

    void _refreshVideo(double *remainingTime);

    void displayVideo();

    virtual int displayWindow();

    void displayVideoImage();

    virtual void displayVideoImageBefore();

    virtual void displayVideoImageAfter(Frame *lastFrame, Rect *rect);

    virtual int uploadTexture(AVFrame *frame, SwsContext *convertContext) = 0;

    virtual void displayVideoAudio();

};


#endif //SPLAYER_VOUT_H
