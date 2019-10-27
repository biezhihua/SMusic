#include <queue/FrameQueue.h>

FrameQueue::FrameQueue(int max_size, int keep_last, PacketQueue *packetQueue) {
    this->packetQueue = packetQueue;
    memset(queue, 0, sizeof(Frame) * FRAME_QUEUE_SIZE);
    maxSize = FFMIN(max_size, FRAME_QUEUE_SIZE);
    keepPreviousFrame = (keep_last != 0);
    for (int i = 0; i < this->maxSize; ++i) {
        queue[i].frame = av_frame_alloc();
    }
    abortRequest = true;
    readIndex = 0;
    writeIndex = 0;
    size = 0;
    readIndexShown = 0;
}

FrameQueue::~FrameQueue() {
    for (int i = 0; i < maxSize; ++i) {
        Frame *vp = &queue[i];
        unrefFrame(vp);
        av_frame_free(&vp->frame);
    }
}

void FrameQueue::start() {
    mutex.lock();
    abortRequest = false;
    condition.signal();
    mutex.unlock();
}

void FrameQueue::abort() {
    mutex.lock();
    abortRequest = true;
    condition.signal();
    mutex.unlock();
}

/// 上一帧
Frame *FrameQueue::peekPreviousFrame() {
    return &queue[readIndex];
}

/// 当前帧
Frame *FrameQueue::peekCurrentFrame() {
    return &queue[(readIndex + readIndexShown) % maxSize];
}

/// 下一帧
Frame *FrameQueue::peekNextFrame() {
    return &queue[(readIndex + readIndexShown + 1) % maxSize];
}

Frame *FrameQueue::peekWritable() {
    mutex.lock();
    while (size >= maxSize && !abortRequest) {
        if (!_condWriteableWait) {
            _condWriteableWait = true;
            if (DEBUG) {
                ALOGD(TAG, "[%s] size = %d maxSize = %d do waiting", __func__, size, maxSize);
            }
        }
        condition.wait(mutex);
    }
    _condWriteableWait = false;
    mutex.unlock();

    if (abortRequest) {
        return nullptr;
    }

    return &queue[writeIndex];
}

void FrameQueue::pushFrame() {
    if (++writeIndex == maxSize) {
        writeIndex = 0;
    }
    mutex.lock();
    size++;
    condition.signal();
    mutex.unlock();
}

void FrameQueue::popFrame() {
    if (keepPreviousFrame && !readIndexShown) {
        readIndexShown = 1;
        return;
    }
    unrefFrame(&queue[readIndex]);
    if (++readIndex == maxSize) {
        readIndex = 0;
    }
    mutex.lock();
    size--;
    condition.signal();
    mutex.unlock();
}

void FrameQueue::flush() {
    while (getFrameSize() > 0) {
        popFrame();
    }
}

int FrameQueue::getFrameSize() { return size - readIndexShown; }

void FrameQueue::unrefFrame(Frame *frame) {
    av_frame_unref(frame->frame);
    avsubtitle_free(&frame->sub);
}

int FrameQueue::isShownIndex() const { return readIndexShown; }

Frame *FrameQueue::peekReadable() {
    // wait until we have a readable a new frame
    mutex.lock();
    while ((size - readIndexShown) <= 0 && !packetQueue->isAbort()) {
        if (!_condReadableWait) {
            _condReadableWait = true;
            if (DEBUG) {
                ALOGD(TAG, "[%s] size = %d maxSize = %d do waiting", __func__, size, maxSize);
            }
        }
        condition.wait(mutex);
    }
    _condReadableWait = false;
    mutex.unlock();

    // abort
    if (packetQueue->isAbort()) {
        return nullptr;
    }

    // 获取queue中一块Frame大小的可写内存
    // 这方法和frameQueuePeek的作用一样， 都是获取待显示的第一帧
    Frame *frame = &queue[(readIndex + readIndexShown) % maxSize];
    return frame;
}

Mutex *FrameQueue::getMutex() { return &mutex; }

int64_t FrameQueue::currentPos() {
    /* return last shown position */
    Frame *frame = &queue[readIndex];
    if (readIndexShown && packetQueue && frame->seekSerial == packetQueue->getLastSeekSerial()) {
        // 返回正在显示的帧的position
        return frame->pos;
    } else {
        return -1;
    }
}