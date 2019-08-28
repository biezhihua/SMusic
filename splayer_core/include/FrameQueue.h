#ifndef SPLAYER_CORE_FRAMEQUEUE_H
#define SPLAYER_CORE_FRAMEQUEUE_H

#include "Mutex.h"
#include "PacketQueue.h"
#include "Define.h"
#include "Frame.h"
#include "Error.h"
#include "Log.h"

#define FRAME_QUEUE_TAG "FrameQueue"

/// 解码后的帧队列
/// https://www.jianshu.com/p/6014de9c47ea
class FrameQueue {
private:
    bool _condWriteableWait = false;
    bool _condReadableWait = false;
public:
    /// queue是存储Frame的数组
    Frame queue[FRAME_QUEUE_SIZE];

    /// 锁对象
    Mutex *mutex;

    /// 指向各自数据包(ES包)的队列
    PacketQueue *packetQueue;

    /// 是写帧数据索引， 相当于是队列的队尾
    int writeIndex;

    /// 是读帧数据索引， 相当于是队列的队首
    int readIndex;

    /// 表示当前是否有帧在显示
    int readIndexShown;

    /// 保持上一个
    int keepLast;

    /// 是存储在这个队列的Frame的数量
    int size;

    /// 是可以存储Frame的最大数量
    int maxSize;

public:

    int unrefItem(Frame *frame);

    int init(PacketQueue *packetQueue, int queueSize, int keepLast);

    int destroy();

    int signal();

    Frame *peekNext();

    Frame *peekNextNext();

    Frame *peek();

    Frame *peekWritable();

    Frame *peekReadable();

    int next();

    int push();

    int numberRemaining();

    /**
     * return last shown position
     */
    int64_t lastPos();
};


#endif //SPLAYER_CORE_FRAMEQUEUE_H
