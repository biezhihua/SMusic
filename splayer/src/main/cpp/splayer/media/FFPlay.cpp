#include "FFPlay.h"

FFPlay::FFPlay() {
    ALOGD(__func__);
    pAvMutex = new Mutex();
    pVfMutex = new Mutex();
    pMsgQueue = new MessageQueue();
}

FFPlay::~FFPlay() {
    ALOGD(__func__);
    delete pPipeline;
    delete pAOut;
    delete pAvMutex;
    delete pVfMutex;
    delete pMsgQueue;
}

void FFPlay::setAOut(AOut *aOut) {
    ALOGD(__func__);
    FFPlay::pAOut = aOut;
}

void FFPlay::setVOut(VOut *vOut) {
    ALOGD(__func__);
    FFPlay::pVOut = vOut;
}

VOut *FFPlay::getVOut() const {
    return pVOut;
}

void FFPlay::setPipeline(Pipeline *pipeline) {
    ALOGD(__func__);
    FFPlay::pPipeline = pipeline;
}

Pipeline *FFPlay::getPipeline() const {
    return pPipeline;
}
