
#include <FrameQueue.h>

#include "FrameQueue.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

int FrameQueue::frameQueueInit(PacketQueue *pPacketQueue, int queueSize, int keepLast) {
    if (!(mutex = new Mutex())) {
        ALOGE("%s create mutex fail", __func__);
        return NEGATIVE(S_NOT_MEMORY);
    }

    packetQueue = pPacketQueue;
    maxSize = FFMIN(queueSize, FRAME_QUEUE_SIZE);
    keepLast = keepLast;

    for (int i = 0; i < maxSize; i++) {
        if (!(queue[i].frame = av_frame_alloc())) {
            return NEGATIVE(ENOMEM);
        }
    }

    ALOGI("%s packetQueue=%p maxSize=%d keepLast=%d",
          __func__,
          packetQueue,
          maxSize,
          keepLast);

    return POSITIVE;
}

int FrameQueue::frameQueueDestroy() {
    for (int i = 0; i < maxSize; i++) {
        Frame *frame = &queue[i];
        av_frame_free(&frame->frame);
    }
    delete mutex;
    mutex = nullptr;
    return POSITIVE;
}

int FrameQueue::frameQueueNbRemaining() {
    /* return the number of undisplayed frames in the queue */
    return size - rIndexShown;
}

int FrameQueue::frameQueueSignal() {
    if (mutex) {
        mutex->mutexUnLock();
        mutex->condSignal();
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(S_ERROR);
}

Frame FrameQueue::frameQueuePeek() {
    return queue[(rIndex + rIndexShown) % maxSize];
}

Frame FrameQueue::frameQueuePeekNext() {
    return queue[(rIndex + rIndexShown + 1) % maxSize];
}

Frame FrameQueue::frameQueuePeekLast() {
    return queue[rIndex];
}

Frame *FrameQueue::frameQueuePeekWritable() {
    /* wait until we have space to put a new frame */
    if (mutex && packetQueue) {
        mutex->mutexLock();
        while (size >= maxSize && !packetQueue->abortRequest) {
            mutex->condWait();
        }
        mutex->mutexUnLock();
        if (packetQueue->abortRequest) {
            return nullptr;
        }
        return &queue[wIndex];
    }
    return nullptr;
}

Frame *FrameQueue::frameQueuePeekReadable() {
    /* wait until we have a readable a new frame */
    if (mutex && packetQueue) {
        mutex->mutexLock();
        while ((size - rIndexShown) <= 0 && !packetQueue->abortRequest) {
            mutex->condWait();
        }
        mutex->mutexUnLock();
        if (packetQueue->abortRequest) {
            return nullptr;
        }
        return &queue[(rIndex + rIndexShown) % maxSize];
    }
    return nullptr;
}

int FrameQueue::frameQueuePush() {
    if (mutex) {
        if (++wIndex == maxSize) {
            wIndex = 0;
        }
        mutex->mutexLock();
        size++;
        mutex->condSignal();
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int FrameQueue::frameQueueNext() {
    if (mutex) {
        if (keepLast && !rIndexShown) {
            rIndexShown = 1;
            return POSITIVE;
        }
        frameQueueUnrefItem(&queue[rIndex]);
        if (++rIndex == maxSize) {
            rIndex = 0;
        }
        mutex->mutexLock();
        size--;
        mutex->condSignal();
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int64_t FrameQueue::frameQueueLastPos() {
    Frame *fp = &queue[rIndex];
    if (rIndexShown && packetQueue && fp->serial == packetQueue->serial) {
        return fp->pos;
    } else {
        return -1;
    }
}


#pragma clang diagnostic pop