#ifndef SPLAYER_ANDROIDPIPELINE_H
#define SPLAYER_ANDROIDPIPELINE_H


#include <Pipeline.h>
#include "AndroidPipelineOpaque.h"

class AndroidPipeline : public Pipeline {

public:
    AndroidPipeline();

    virtual ~AndroidPipeline();

    void close() override;

    AOut *openAudioOutput() override;

    PipelineNode *openVideoDecoder() override;

    PipelineOpaque *createOpaque() override;

    PipelineNode *openAudioDecoder() override;
};


#endif //SPLAYER_ANDROIDPIPELINE_H
