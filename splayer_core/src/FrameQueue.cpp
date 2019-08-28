#include "FrameQueue.h"

int FrameQueue::init(PacketQueue *packetQueue, int queueSize, int keepLast) {

    // 创建互斥量
    if (!(mutex = new Mutex())) {
        ALOGE(FRAME_QUEUE_TAG, "%s create mutex fail", __func__);
        return NEGATIVE(S_NOT_MEMORY);
    }

    FrameQueue::packetQueue = packetQueue;

    maxSize = FFMIN(queueSize, FRAME_QUEUE_SIZE);

    FrameQueue::keepLast = keepLast;

    // 为帧队列中的各个Frame创建AvFrame
    for (int i = 0; i < maxSize; i++) {
        if (!(queue[i].frame = av_frame_alloc())) {
            return NEGATIVE(ENOMEM);
        }
    }

    ALOGD(FRAME_QUEUE_TAG, "%s packetQueue = %p maxSize[%d] = %d keepLast = %d",
          __func__,
          FrameQueue::packetQueue,
          FRAME_QUEUE_SIZE,
          maxSize,
          FrameQueue::keepLast);

    return POSITIVE;
}

int FrameQueue::destroy() {
    // 释放Frame，释放互斥锁和互斥量
    for (int i = 0; i < maxSize; i++) {
        Frame *frame = &queue[i];
        if (frame) {
            av_frame_free(&frame->frame);
        }
    }
    delete mutex;
    mutex = nullptr;
    return POSITIVE;
}

int FrameQueue::numberRemaining() {
    /* return the number of undisplayed frames in the packetQueue */
    // 返回队列中待显示帧的数目
    return size - readIndexShown;
}

int FrameQueue::signal() {
    if (mutex) {
        mutex->mutexUnLock();
        mutex->condSignal();
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(S_ERROR);
}

Frame *FrameQueue::peek() {
    // 获取当前播放器显示的帧
    return &queue[readIndex];
}

Frame *FrameQueue::peekNext() {
    // 获取待显示的第一个帧
    return &queue[(readIndex + readIndexShown) % maxSize];
}

Frame *FrameQueue::peekNextNext() {
    // 获取待显示的第二个帧
    return &queue[(readIndex + readIndexShown + 1) % maxSize];
}

Frame *FrameQueue::peekWritable() {
    if (mutex && packetQueue) {
        // wait until we have space to put a new frame
        mutex->mutexLock();
        while (size >= maxSize && !packetQueue->abortRequest) {
            if (!_condWriteableWait) {
                _condWriteableWait = true;
                ALOGW(FRAME_QUEUE_TAG, "%s size = %d maxSize = %d do waiting", __func__, size, maxSize);
            }
            mutex->condWait();
        }
        _condWriteableWait = false;
        mutex->mutexUnLock();

        // abort
        if (packetQueue->abortRequest) {
            return nullptr;
        }

        // 获取queue中一块Frame大小的可写内存
        return &queue[writeIndex];
    }
    return nullptr;
}

Frame *FrameQueue::peekReadable() {
    if (mutex && packetQueue) {

        // wait until we have a readable a new frame
        mutex->mutexLock();
        while ((size - readIndexShown) <= 0 && !packetQueue->abortRequest) {
            if (!_condReadableWait) {
                _condReadableWait = true;
                ALOGW(FRAME_QUEUE_TAG, "%s size = %d maxSize = %d do waiting", __func__, size, maxSize);
            }
            mutex->condWait();
        }
        _condReadableWait = false;
        mutex->mutexUnLock();

        // abort
        if (packetQueue->abortRequest) {
            return nullptr;
        }

        // 获取queue中一块Frame大小的可写内存
        // 这方法和frameQueuePeek的作用一样， 都是获取待显示的第一帧
        return &queue[(readIndex + readIndexShown) % maxSize];
    }
    return nullptr;
}

int FrameQueue::push() {
    if (mutex) {
        // 推入一帧数据， 其实数据已经在调用这个方法前填充进去了，
        // 这个方法的作用是将队列的写索引(也就是队尾)向后移，
        // 还有将这个队列中的Frame的数量加一。
        if (++writeIndex == maxSize) {
            writeIndex = 0;
        }
        mutex->mutexLock();
        size++;
        mutex->condSignal();
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int FrameQueue::next() {
    if (mutex) {
        // 将读索引(队头)后移一位， 还有将这个队列中的Frame的数量减一
        if (keepLast && !readIndexShown) {
            readIndexShown = 1;
            return POSITIVE;
        }

        unrefItem(&queue[readIndex]);

        if (++readIndex == maxSize) {
            readIndex = 0;
        }

        mutex->mutexLock();
        size--;
        mutex->condSignal();
        mutex->mutexUnLock();

        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int64_t FrameQueue::lastPos() {
    /* return last shown position */
    Frame *fp = &queue[readIndex];
    if (readIndexShown && packetQueue && fp->serial == packetQueue->serial) {
        // 返回正在显示的帧的position
        return fp->pos;
    } else {
        return -1;
    }
}

int FrameQueue::unrefItem(Frame *frame) {
    if (frame) {
        // 取消引用帧引用的所有缓冲区并重置帧字段，释放给定字幕结构中的所有已分配数据。
        av_frame_unref(frame->frame);
        avsubtitle_free(&frame->subtitle);
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

