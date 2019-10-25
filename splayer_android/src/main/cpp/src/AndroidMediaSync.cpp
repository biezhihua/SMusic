#include <AndroidMediaSync.h>

void AndroidMediaSync::start(VideoDecoder *videoDecoder, AudioDecoder *audioDecoder) {
    MediaSync::start(videoDecoder, audioDecoder);
    if (DEBUG) {
        ALOGD(TAG, __func__);
    }
    if (playerMutex) {
        playerMutex->lock();
    }
    if (videoDecoder && !syncThread) {
        syncThread = new Thread(this);
        syncThread->start();
    }
    isQuit = false;
    if (playerMutex) {
        playerMutex->unlock();
    }
}

void AndroidMediaSync::stop() {
    MediaSync::stop();
    if (DEBUG) {
        ALOGD(TAG, __func__);
    }
    if (playerMutex) {
        playerMutex->lock();
    }
    isQuit = true;
    if (syncThread) {
        syncThread->join();
        delete syncThread;
        syncThread = nullptr;
    }
    if (playerMutex) {
        playerMutex->unlock();
    }

}

void AndroidMediaSync::run() {
    resetRemainingTime();
    while (!isQuit) {
        if (playerMutex) {
            playerMutex->lock();
        }
        refreshVideo();
        if (playerMutex) {
            playerMutex->unlock();
        }
    }
}
