//
// Created by biezhihua on 2019-06-16.
//

#ifndef SPLAYER_ANDROIDPIPELINE_H
#define SPLAYER_ANDROIDPIPELINE_H


#include "../media/Log.h"
#include "AndroidPipelineOpaque.h"
#include "../media/Pipeline.h"

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
