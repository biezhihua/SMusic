
#ifndef SPLAYER_PIPELINE_H
#define SPLAYER_PIPELINE_H

#include "Log.h"
#include "AOut.h"
#include "VOut.h"
#include "PipelineNode.h"

class Pipeline {

private:

    VOut *vOut;

public:
    Pipeline();

    virtual ~Pipeline();

    // destroy
    virtual void close() = 0;

    virtual AOut *openAudioOutput() = 0;

    virtual PipelineNode *openAudioDecoder() = 0;

    virtual PipelineNode *openVideoDecoder() = 0;

    void setVOut(VOut *vOut);

};


#endif //SPLAYER_PIPELINE_H
