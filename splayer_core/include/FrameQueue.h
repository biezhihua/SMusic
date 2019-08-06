#ifndef SPLAYER_MAC_FRAMEQUEUE_H
#define SPLAYER_MAC_FRAMEQUEUE_H

#include "Mutex.h"
#include "PacketQueue.h"
#include "Define.h"
#include "Frame.h"
#include "Error.h"
#include "Log.h"

/**
 * https://www.jianshu.com/p/6014de9c47ea
 */
class FrameQueue {
public:
    // queue是存储Frame的数组
    Frame queue[FRAME_QUEUE_SIZE];

    // 锁对象
    Mutex *mutex;

    // 指向各自数据包(ES包)的队列
    PacketQueue *packetQueue;

    // 是写帧数据索引， 相当于是队列的队尾
    int writeIndex;

    // 是读帧数据索引， 相当于是队列的队首
    int readIndex;

    // 表示当前是否有帧在显示
    int readIndexShown;

    // 这个变量的含义，据我分析， 是用来判断队列是否保留正在显示的帧(Frame)
    int keepLast;

    // 是存储在这个队列的Frame的数量
    int size;

    // 是可以存储Frame的最大数量
    int maxSize;

public:

    int frameQueueUnrefItem(Frame *frame);

    int frameQueueInit(PacketQueue *pPacketQueue, int queueSize, int keepLast);

    int frameQueueDestroy();

    int frameQueueSignal();

    Frame *frameQueuePeek();

    Frame *frameQueuePeekNext();

    Frame *frameQueuePeekLast();

    Frame *frameQueuePeekWritable();

    Frame *frameQueuePeekReadable();

    int frameQueueNext();

    int frameQueuePush();

    int frameQueueNumberRemaining();

    /**
     * return last shown position
     */
    int64_t frameQueueLastPos();
};


#endif //SPLAYER_MAC_FRAMEQUEUE_H
