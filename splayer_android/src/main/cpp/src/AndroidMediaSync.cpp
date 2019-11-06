#include "AndroidMediaSync.h"

void AndroidMediaSync::start(VideoDecoder *videoDecoder, AudioDecoder *audioDecoder) {
    MediaSync::start(videoDecoder, audioDecoder);
    mutex.lock();
    isQuit = false;
    mutex.unlock();
    if (videoDecoder && !syncThread) {
        syncThread = new Thread(this);
        syncThread->start();
        if (DEBUG) {
            ALOGD(TAG, "[%s] sync thread already started", __func__);
        }
    }
}

void AndroidMediaSync::stop() {
    MediaSync::stop();
    mutex.lock();
    isQuit = true;
    mutex.unlock();
    if (syncThread) {
        syncThread->join();
        delete syncThread;
        syncThread = nullptr;
        if (DEBUG) {
            ALOGD(TAG, "[%s] sync thread already die", __func__);
        }
    }
}

void AndroidMediaSync::run() {
    int ret = 0;
    resetRemainingTime();
    if (DEBUG) {
        ALOGD(TAG, "[%s] sync thread refresh video start", __func__);
    }
    while (!isQuit) {
        if ((ret = refreshVideo()) < 0) {
            ALOGE(TAG, "[%s] refresh video exit, ret = %d", __func__, ret);
            isQuit = true;
        }
    }
    if (DEBUG) {
        ALOGD(TAG, "[%s] sync thread refresh video end", __func__);
    }
}
