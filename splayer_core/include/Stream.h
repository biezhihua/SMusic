#ifndef SPLAYER_PIPELINE_H
#define SPLAYER_PIPELINE_H

class Surface;

class FFPlay;

#include "Log.h"
#include "Audio.h"
#include "Surface.h"
#include "FFPlay.h"
#include "Error.h"

extern "C" {
#include <libavformat/avformat.h>
}

class Stream {

private:
    FFPlay *play = nullptr;
    Surface *surface = nullptr;

public:
    Stream();

    virtual ~Stream();

    virtual int create();

    virtual int destroy();

    void setSurface(Surface *surface);

    void setPlay(FFPlay *play);

};


#endif //SPLAYER_PIPELINE_H
