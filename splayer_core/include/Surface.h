#ifndef SPLAYER_VOUT_H
#define SPLAYER_VOUT_H

class FFPlay;

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "Log.h"
#include "FFPlay.h"
#include "Mutex.h"
#include "Options.h"
#include "Rect.h"
#include "Frame.h"

extern "C" {
#include <libavutil/rational.h>
}

#define SURFACE_TAG "Surface"

class Surface {

protected:
    FFPlay *play = nullptr;
    Mutex *mutex = nullptr;
    Options *options = nullptr;
    Stream *stream = nullptr;
public:
    Surface();

    virtual ~Surface();

    virtual int create() = 0;

    virtual int destroy() = 0;

    virtual void setWindowSize(int width, int height, AVRational rational);

    void setPlay(FFPlay *play);

    virtual void displayWindow();

    virtual void displayVideoImage();

    virtual Options *getOptions() const;

    void setOptions(Options *options);

    Stream *getStream() const;

    void setStream(Stream *stream);

    virtual void doExit();

    void calculateDisplayRect(Rect *rect, int screenXLeft, int screenYTop, int screenWidth, int screenHeight, int pictureWidth, int pictureHeight, AVRational picSar);

    virtual void displayVideoImageBefore();

    virtual void displayVideoImageAfter(Frame *lastFrame, Rect *rect);

    virtual int uploadTexture(AVFrame *frame, SwsContext *convertContext) = 0;
};


#endif //SPLAYER_VOUT_H
