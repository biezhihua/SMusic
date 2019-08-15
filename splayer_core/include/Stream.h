#ifndef SPLAYER_PIPELINE_H
#define SPLAYER_PIPELINE_H

#include "Log.h"
#include "Audio.h"
#include "Surface.h"
#include "PipelineNode.h"

class Stream {

private:

    Surface *surface;

public:
    Stream();

    virtual ~Stream();

    virtual int create() = 0;

    virtual int destroy() = 0;

    void setSurface(Surface *surface);

};


#endif //SPLAYER_PIPELINE_H
