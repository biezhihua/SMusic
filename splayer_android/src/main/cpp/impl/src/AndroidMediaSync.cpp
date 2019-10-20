#include <AndroidMediaSync.h>

void AndroidMediaSync::start(VideoDecoder *videoDecoder, AudioDecoder *audioDecoder) {
    MediaSync::start(videoDecoder, audioDecoder);
    if (playerMutex) {
        playerMutex->lock();
    }
    if (videoDecoder && !syncThread) {
        syncThread = new Thread(this);
        syncThread->start();
    }
    if (playerMutex) {
        playerMutex->unlock();
    }
}

void AndroidMediaSync::stop() {
    MediaSync::stop();
    if (playerMutex) {
        playerMutex->lock();
    }
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
    MediaSync::run();
    if (playerMutex) {
        playerMutex->lock();
    }
    refreshVideo();
    if (playerMutex) {
        playerMutex->unlock();
    }
}
