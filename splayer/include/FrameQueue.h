//
// Created by biezhihua on 2019-07-16.
//

#ifndef ANDROID2_SPLAYER_FRAMEQUEUE_H
#define ANDROID2_SPLAYER_FRAMEQUEUE_H

#include "Common.h"
#include "Frame.h"
#include "PacketQueue.h"
#include "Mutex.h"

class FrameQueue {
public:
    Frame queue[FRAME_QUEUE_SIZE];
    int rindex;
    int windex;
    int size;
    int max_size;
    int keep_last;
    int rindex_shown;
    Mutex *mutex;
    PacketQueue *pktq;
};

#endif //ANDROID_SPLAYER_FRAMEQUEUE_H
