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
public:
    Surface();

    virtual ~Surface();

    virtual int create() = 0;

    virtual int destroy() = 0;

    virtual void setWindowSize(int width, int height, AVRational rational);

    void setStream(Stream *stream);

    virtual void displayWindow(int width, int height);

    virtual void displayVideoImage();

    virtual Options *getOptions() const;

    void setOptions(Options *options);

    void calculateDisplayRect(Rect *rect, int screenXLeft, int screenYTop, int screenWidth, int screenHeight, int pictureWidth, int pictureHeight, AVRational picSar);

    virtual void displayVideoImageBefore();

    virtual void displayVideoImageAfter(Frame *lastFrame, Rect *rect);

    virtual int uploadTexture(AVFrame *frame, SwsContext *convertContext) = 0;

    MediaPlayer *getMediaPlayer() const;

    void setMediaPlayer(MediaPlayer *mediaPlayer);
};


#endif //SPLAYER_VOUT_H
