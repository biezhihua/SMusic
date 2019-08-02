#include "MacPipeline.h"

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
