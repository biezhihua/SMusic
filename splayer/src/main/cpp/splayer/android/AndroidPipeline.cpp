#include "AndroidPipeline.h"
#include "AndroidPipelineNode.h"

AndroidPipeline::AndroidPipeline() {
    ALOGD(__func__);
}

AndroidPipeline::~AndroidPipeline() {
    ALOGD(__func__);
}

void AndroidPipeline::close() {
    ALOGD(__func__);
}

AOut *AndroidPipeline::openAudioOutput() {
    ALOGD(__func__);
    return nullptr;
}

PipelineNode *AndroidPipeline::openVideoDecoder() {
    ALOGD(__func__);
    return nullptr;
}

PipelineOpaque *AndroidPipeline::createOpaque() {
    ALOGD(__func__);
    return new AndroidPipelineOpaque();
}

PipelineNode *AndroidPipeline::openAudioDecoder() {
    ALOGD(__func__);
    return new AndroidPipelineNode();
}
