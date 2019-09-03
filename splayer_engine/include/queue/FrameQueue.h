#ifndef MEDIAPLAYER_FRAMEQUEUE_H
#define MEDIAPLAYER_FRAMEQUEUE_H

#include <common/Mutex.h>
#include <common/Condition.h>

extern "C" {
#include <libavcodec/avcodec.h>
};

#define FRAME_QUEUE_SIZE 10

typedef struct Frame {

    AVFrame *frame;

    AVSubtitle sub;

    double pts;           /* presentation timestamp for the frame */

    double duration;      /* estimated duration of the frame */

    int width;

    int height;

    int format;

    int uploaded;

} Frame;

/// 解码后的帧队列
/// https://www.jianshu.com/p/6014de9c47ea
class FrameQueue {

public:
    FrameQueue(int max_size, int keep_last);

    virtual ~FrameQueue();

    void start();

    void abort();

    Frame *currentFrame();

    Frame *nextFrame();

    Frame *lastFrame();

    Frame *peekWritable();

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
};


#endif //MEDIAPLAYER_FRAMEQUEUE_H
