//
// Created by biezhihua on 2019/2/4.
//

#ifndef SMUSIC_S_QUEUE_H
#define SMUSIC_S_QUEUE_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};

#include <queue>
#include <pthread.h>
#include "SLog.h"
#include "SError.h"

using namespace std;

class SQueue {

private:

    queue<AVPacket *> *pQueuePacket;
    pthread_mutex_t mutexPacketMutex;
    pthread_cond_t condPacketMutex;

public:

    SQueue();

    ~SQueue();

    /**
     * Thread Safe
     */
    int putAvPacket(AVPacket *pPacket);

    /**
     * Thread Safe
     */
    int getAvPacket(AVPacket *pPacket);

    int getSize();

    void threadLock();

    void threadUnlock();

    void clear();
};


#endif //SMUSIC_S_QUEUE_H