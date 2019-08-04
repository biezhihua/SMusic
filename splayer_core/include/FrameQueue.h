#ifndef SPLAYER_MAC_FRAMEQUEUE_H
#define SPLAYER_MAC_FRAMEQUEUE_H

#include "Mutex.h"
#include "PacketQueue.h"
#include "Define.h"
#include "Frame.h"

class FrameQueue {

public:
    Frame queue[FRAME_QUEUE_SIZE];
    Mutex *mutex;
    PacketQueue *packetQueue;
    int maxSize;
    int keepLast;
};


#endif //SPLAYER_MAC_FRAMEQUEUE_H
