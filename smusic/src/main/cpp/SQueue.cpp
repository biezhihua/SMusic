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
        pthread_mutex_lock(&mutexPacket);
        pQueuePacket->push(pPacket);
        LOGD("put AVPacket to queue, %d", pQueuePacket->size());
        pthread_cond_signal(&condPacket);
        pthread_mutex_unlock(&mutexPacket);
        return 0;
    }
    return -1;
}

int SQueue::getAvPacket(AVPacket *pPacket) {
    pthread_mutex_lock(&mutexPacket);
    if (pQueuePacket != NULL && pQueuePacket->size() > 0) {
        AVPacket *avPacket = pQueuePacket->front();
        if (av_packet_ref(pPacket, avPacket) == 0) {
            pQueuePacket->pop();
        }
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
        LOGD("get AVPacket from queue, %d", pQueuePacket->size());
    } else {
        pthread_cond_wait(&condPacket, &mutexPacket);
    }
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int SQueue::getSize() {
    if (pQueuePacket != NULL) {
        return pQueuePacket->size();
    }
    return 0;
}
