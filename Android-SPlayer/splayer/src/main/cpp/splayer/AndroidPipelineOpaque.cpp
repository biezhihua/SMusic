#include "AndroidPipelineOpaque.h"

AndroidPipelineOpaque::AndroidPipelineOpaque() : PipelineOpaque() {
    ALOGD(__func__);
    pSurfaceMutex = new Mutex();
}

AndroidPipelineOpaque::~AndroidPipelineOpaque() {
    ALOGD(__func__);
    delete pSurfaceMutex;
}
