#ifndef SPLAYER_COMMAND_MACPIPELINE_H
#define SPLAYER_COMMAND_MACPIPELINE_H

#include "Pipeline.h"

class MacPipeline : public Pipeline {

public:

    void close() override;

    Audio *openAudioOutput() override;

    PipelineNode *openAudioDecoder() override;

    PipelineNode *openVideoDecoder() override;


};


#endif //SPLAYER_COMMAND_MACPIPELINE_H
