#include "queue/FrameQueue.h"

FrameQueue::FrameQueue(int max_size, int keep_last) {
    memset(queue, 0, sizeof(Frame) * FRAME_QUEUE_SIZE);
    maxSize = FFMIN(max_size, FRAME_QUEUE_SIZE);
    keepLast = (keep_last != 0);
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

Frame *FrameQueue::nextFrame() {
    return &queue[(readIndex + readIndexShown) % maxSize];
}

Frame *FrameQueue::next2Frame() {
    return &queue[(readIndex + readIndexShown + 1) % maxSize];
}

Frame *FrameQueue::currentFrame() {
    return &queue[readIndex];
}

Frame *FrameQueue::peekWritable() {
    mutex.lock();
    while (size >= maxSize && !abortRequest) {
        condition.wait(mutex);
    }
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
    if (keepLast && !readIndexShown) {
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

int FrameQueue::getFrameSize() {
    return size - readIndexShown;
}

void FrameQueue::unrefFrame(Frame *vp) {
    av_frame_unref(vp->frame);
    avsubtitle_free(&vp->sub);
}

int FrameQueue::getShowIndex() const {
    return readIndexShown;
}
