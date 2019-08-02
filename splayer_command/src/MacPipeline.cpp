#include "MacPipeline.h"
#include "MacPipelineOpaque.h"

PipelineOpaque *MacPipeline::createOpaque() {
    return new MacPipelineOpaque();
}

void MacPipeline::close() {

}

AOut *MacPipeline::openAudioOutput() {
    return nullptr;
}

PipelineNode *MacPipeline::openAudioDecoder() {
    return nullptr;
}

PipelineNode *MacPipeline::openVideoDecoder() {
    return nullptr;
}
