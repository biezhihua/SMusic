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

using namespace std;

class SQueue {

private:

    queue<AVPacket *> *pQueuePacket;
    pthread_mutex_t mutexPacket;
    pthread_cond_t condPacket;

public:

    SQueue();

    ~SQueue();

    int putAvPacket(AVPacket *pPacket);

    int getAvPacket(AVPacket *pPacket);

    int getSize();

};


#endif //SMUSIC_S_QUEUE_H
