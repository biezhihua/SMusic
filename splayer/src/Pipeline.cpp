#include "../include/Pipeline.h"

Pipeline::Pipeline() {
    ALOGD(__func__);
}

Pipeline::~Pipeline() {
    ALOGD(__func__);
}

void Pipeline::setOpaque(PipelineOpaque *opaque) {
    Pipeline::opaque = opaque;
}
