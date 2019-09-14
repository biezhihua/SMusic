#ifndef MEDIAPLAYER_FRAMEQUEUE_H
#define MEDIAPLAYER_FRAMEQUEUE_H

#include <common/Mutex.h>
#include <common/Condition.h>
#include <common/Log.h>
#include <queue/PacketQueue.h>

extern "C" {
#include <libavcodec/avcodec.h>
};

#define FRAME_QUEUE_SIZE 10

typedef struct Frame {

    AVFrame *frame;

    AVSubtitle sub;

    int seekSerial;

    double pts;           /* presentation timestamp for the frame */

    double duration;      /* estimated duration of the frame */

    int64_t pos;          /* byte position of the frame in the input file */

    int width;

    int height;

    int format;

    AVRational sar;

    int uploaded;

    int flip_v;

} Frame;

/// 解码后的帧队列
/// https://www.jianshu.com/p/6014de9c47ea
class FrameQueue {
    const char *const TAG = "FrameQueue";

private:
    bool _condWriteableWait = false;
    bool _condReadableWait = false;

public:
    FrameQueue(int max_size, int keep_last, PacketQueue *packetQueue);

    virtual ~FrameQueue();

    void start();

    void abort();

    Frame *nextFrame();

    Frame *next2Frame();

    Frame *currentFrame();

    Frame *peekWritable();

    Frame *peekReadable();

    void pushFrame();

    void popFrame();

    void flush();

    int getFrameSize();

    int getShowIndex() const;

private:
    void unrefFrame(Frame *vp);

private:

    /// 锁对象
    Mutex mutex;

    /// 条件对象
    Condition condition;

    bool abortRequest;

    /// queue是存储Frame的数组
    Frame queue[FRAME_QUEUE_SIZE];

    /// 是读帧数据索引， 相当于是队列的队首
    int readIndex;

    /// 是写帧数据索引， 相当于是队列的队尾
    int writeIndex;

    /// 是存储在这个队列的Frame的数量
    int size;

    /// 是可以存储Frame的最大数量
    int maxSize;

    /// 保持上一个
    int keepLast;

    /// 表示当前是否有帧在显示
    int readIndexShown;

    /// 指向各自数据包(ES包)的队列
    PacketQueue *packetQueue;
};


#endif //MEDIAPLAYER_FRAMEQUEUE_H
