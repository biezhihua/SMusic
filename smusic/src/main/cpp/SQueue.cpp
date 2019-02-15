//
// Created by biezhihua on 2019/2/4.
//


#include "SQueue.h"

SQueue::SQueue() {
    pthread_cond_init(&condPacket, NULL);
    pthread_mutex_init(&mutexPacket, NULL);
    pQueuePacket = new queue<AVPacket *>();
}

SQueue::~SQueue() {
    delete pQueuePacket;
    pQueuePacket = NULL;
}

int SQueue::putAvPacket(AVPacket *pPacket) {
    if (pQueuePacket != NULL && pPacket != NULL) {
        threadLock();
        pQueuePacket->push(pPacket);
        LOGD("SQueue: putAvPacket: size = %d", pQueuePacket->size());
        pthread_cond_signal(&condPacket);
        threadUnlock();
        return S_SUCCESS;
    }
    return S_ERROR;
}

int SQueue::getAvPacket(AVPacket *pPacket) {
    if (pQueuePacket != NULL && pQueuePacket->size() > 0) {
        AVPacket *avPacket = pQueuePacket->front();
        if (av_packet_ref(pPacket, avPacket) == 0) {
            pQueuePacket->pop();
        }
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
        LOGD("SQueue: getAvPacket: size = %d", pQueuePacket->size());
        return S_SUCCESS;
    } else {
        pthread_cond_wait(&condPacket, &mutexPacket);
        return S_ERROR_CONTINUE;
    }
}

void SQueue::threadUnlock()  { pthread_mutex_unlock(&mutexPacket); }

void SQueue::threadLock()  { pthread_mutex_lock(&mutexPacket); }

int SQueue::getSize() {
    if (pQueuePacket != NULL) {
        return pQueuePacket->size();
    }
    return 0;
}
