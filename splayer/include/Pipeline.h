
#ifndef SPLAYER_PIPELINE_H
#define SPLAYER_PIPELINE_H

#include "Log.h"
#include "PipelineOpaque.h"
#include "AOut.h"
#include "PipelineNode.h"

class Pipeline {

private:
    PipelineOpaque *opaque = nullptr;

public:
    Pipeline();

    virtual ~Pipeline();

    void setOpaque(PipelineOpaque *opaque);

    virtual PipelineOpaque *createOpaque() = 0;

    // destroy
    virtual void close() = 0;

    virtual AOut *openAudioOutput() = 0;

    virtual PipelineNode *openAudioDecoder() = 0;

    virtual PipelineNode *openVideoDecoder() = 0;

};


#endif //SPLAYER_PIPELINE_H
