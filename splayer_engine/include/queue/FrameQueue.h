#ifndef MEDIAPLAYER_FRAMEQUEUE_H
#define MEDIAPLAYER_FRAMEQUEUE_H

#include <common/Mutex.h>
#include <common/Condition.h>
#include <common/Log.h>
#include <queue/PacketQueue.h>
#include <player/PlayerState.h>

extern "C" {
#include <libavcodec/avcodec.h>
};


/// 解码帧结构
/// Common struct for handling all types of decoded data and allocated render buffers.
typedef struct Frame {

    /// 帧数据
    AVFrame *frame;

    /// 字幕数据
    AVSubtitle sub;

    /// 序列，作seek时使用，作为区分前后帧序列
    int seekSerial;

    /// 帧的显示时间戳
    /// presentation timestamp for the frame
    double pts;

    /// 帧显示时长
    /// estimated duration of the frame
    double duration;

    /// 帧在文件中的字节位置
    /// byte position of the frame in the input file
    int64_t pos;

    /// 帧宽度
    int width;

    /// 帧高度
    int height;

    /// 帧格式
    int format;

    /// 采样率比率
    AVRational sampleAspectRatio;

    /// 上载
    int uploaded;

    /// 反转
    int flipVertical;

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

    Mutex *getMutex();

    int64_t lastPos();

private:
    void unrefFrame(Frame *frame);

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
