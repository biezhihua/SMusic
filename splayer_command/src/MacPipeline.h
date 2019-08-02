//
// Created by biezhihua on 2019/8/2.
//

#ifndef SPLAYER_COMMAND_MACPIPELINE_H
#define SPLAYER_COMMAND_MACPIPELINE_H


#include <Pipeline.h>

class MacPipeline : public Pipeline {

public:

    void close() override;

    AOut *openAudioOutput() override;

    PipelineNode *openAudioDecoder() override;

    PipelineNode *openVideoDecoder() override;
};


#endif //SPLAYER_COMMAND_MACPIPELINE_H
