//
// Created by biezhihua on 2019-06-16.
//

#include "PipelineOpaque.h"

PipelineOpaque::PipelineOpaque() {
    ALOGD(__func__);
    pSurfaceMutex = new Mutex();
    leftVolume = 1.0f;
    rightVolume = 1.0f;
}

PipelineOpaque::~PipelineOpaque() {
    ALOGD(__func__);
    delete pSurfaceMutex;
}

void PipelineOpaque::setVOut(VOut *vOut) {
    PipelineOpaque::pVOut = vOut;
}

