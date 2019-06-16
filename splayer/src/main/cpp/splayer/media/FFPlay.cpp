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

void FFPlay::setPipeline(Pipeline *pipeline) {
    ALOGD(__func__);
    FFPlay::pPipeline = pipeline;
}

MessageQueue *FFPlay::getMsgQueue() const {
    return pMsgQueue;
}

int FFPlay::stop() {
    // TODO
    return EXIT_FAILURE;
}

int FFPlay::shutdown() {
    // TODO
    waitStop();
    return EXIT_FAILURE;
}

int FFPlay::waitStop() {
    // TODO
    return EXIT_FAILURE;
}

int FFPlay::prepareAsync(const char *fileName) {
    ALOGD("%s fileName=%s", __func__, fileName);
    return EXIT_FAILURE;
}
