#include "PipelineOpaque.h"

PipelineOpaque::PipelineOpaque() {
    ALOGD(__func__);

}

PipelineOpaque::~PipelineOpaque() {
    ALOGD(__func__);

}

void PipelineOpaque::setVOut(VOut *vOut) {
    PipelineOpaque::vOut = vOut;
}

