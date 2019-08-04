#ifndef SPLAYER_MAC_FRAMEQUEUE_H
#define SPLAYER_MAC_FRAMEQUEUE_H

#include "Mutex.h"
#include "PacketQueue.h"
#include "Define.h"
#include "Frame.h"
#include "Error.h"
#include "Log.h"

class FrameQueue {
public:
    Frame queue[FRAME_QUEUE_SIZE];
    Mutex *mutex;
    PacketQueue *packetQueue;
    int maxSize;
    int keepLast;
    int rIndex;
    int wIndex;
    int size;
    int rIndexShown;

public:

    int frameQueueUnrefItem(Frame *frame);

    int frameQueueInit(PacketQueue *pPacketQueue, int queueSize, int keepLast);

    int frameQueueDestroy();

    int frameQueueSignal();

    Frame frameQueuePeek();

    Frame frameQueuePeekNext();

    Frame frameQueuePeekLast();

    Frame *frameQueuePeekWritable();

    Frame *frameQueuePeekReadable();

    int frameQueueNext();

    int frameQueuePush();

    int frameQueueNbRemaining();

    /**
     * return last shown position
     */
    int64_t frameQueueLastPos();
};


#endif //SPLAYER_MAC_FRAMEQUEUE_H
