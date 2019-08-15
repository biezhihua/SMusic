#ifndef SPLAYER_PIPELINE_H
#define SPLAYER_PIPELINE_H

class Surface;

class FFPlay;

#include "Log.h"
#include "Audio.h"
#include "Surface.h"
#include "FFPlay.h"
#include "PipelineNode.h"

class Stream {

private:
    FFPlay *play = nullptr;
    Surface *surface = nullptr;

public:
    Stream();

    virtual ~Stream();

    virtual int create() = 0;

    virtual int destroy() = 0;

    void setSurface(Surface *surface);

    void setPlay(FFPlay *play);

};


#endif //SPLAYER_PIPELINE_H
