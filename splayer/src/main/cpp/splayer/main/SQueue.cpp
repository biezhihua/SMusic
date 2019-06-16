#include "SQueue.h"

SQueue::SQueue() {
    pthread_cond_init(&condPacketMutex, nullptr);
    pthread_mutex_init(&mutexPacketMutex, nullptr);
    pQueuePacket = new queue<AVPacket *>();
}

SQueue::~SQueue() {
    delete pQueuePacket;
    pQueuePacket = nullptr;
    pthread_cond_destroy(&condPacketMutex);
    pthread_mutex_destroy(&mutexPacketMutex);
}

int SQueue::putAvPacket(AVPacket *pPacket) {
    if (pQueuePacket != nullptr && pPacket != nullptr) {
        threadLock();
        pQueuePacket->push(pPacket);
        // LOGD("SQueue: putAvPacket: size = %d", pQueuePacket->size());
        pthread_cond_signal(&condPacketMutex);
        threadUnlock();
        return S_SUCCESS;
    }
    return S_ERROR;
}

int SQueue::getAvPacket(AVPacket *pPacket) {
    if (pQueuePacket != nullptr && pQueuePacket->size() > 0) {
        AVPacket *avPacket = pQueuePacket->front();
        if (av_packet_ref(pPacket, avPacket) == 0) {
            pQueuePacket->pop();
        }
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = nullptr;
        // LOGD("SQueue: getAvPacket: size = %d", pQueuePacket->size());
        return S_SUCCESS;
    } else {
        pthread_cond_wait(&condPacketMutex, &mutexPacketMutex);
        return S_FUNCTION_CONTINUE;
    }
}

void SQueue::threadUnlock() { pthread_mutex_unlock(&mutexPacketMutex); }

void SQueue::threadLock() { pthread_mutex_lock(&mutexPacketMutex); }

int SQueue::getSize() {
    if (pQueuePacket != nullptr) {
        return pQueuePacket->size();
    }
    return 0;
}

void SQueue::clear() {
    pthread_cond_signal(&condPacketMutex);
    pthread_mutex_lock(&mutexPacketMutex);
    if (pQueuePacket != nullptr) {
        while (!pQueuePacket->empty()) {
            AVPacket *avPacket = pQueuePacket->front();
            if (avPacket != nullptr) {
                pQueuePacket->pop();
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = nullptr;
            }
        }
    }
    pthread_mutex_unlock(&mutexPacketMutex);
}
