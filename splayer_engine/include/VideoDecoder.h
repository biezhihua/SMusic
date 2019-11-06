#ifndef ENGINE_VIDEO_DECODER_H
#define ENGINE_VIDEO_DECODER_H

#include "MediaDecoder.h"
#include "PlayerInfoStatus.h"
#include "MediaClock.h"

class VideoDecoder : public MediaDecoder {
    const char *const TAG = "[MP][NATIVE][VideoDecoder]";
public:
    VideoDecoder(AVFormatContext *formatCtx,
                 AVCodecContext *avctx,
                 AVStream *stream,
                 int streamIndex,
                 PlayerInfoStatus *playerState,
                 AVPacket *flushPacket,
                 Condition *pCondition,
                 AVDictionary *opts,
                 MessageCenter *messageCenter);

    virtual ~VideoDecoder();

    void setMasterClock(MediaClock *masterClock);

    void start() override;

    void stop() override;

    void flush() override;

    int getFrameSize();

    int getRotate();

    FrameQueue *getFrameQueue();

    void run() override;

    bool isFinished() override;

    int64_t getFrameQueueLastPos();

private:

    /// 解复用上下文
    AVFormatContext *formatContext;

    /// 帧队列
    FrameQueue *frameQueue;

    /// 旋转角度
    int rotate;

    /// 解码线程
    Thread *decodeThread;

    /// 主时钟
    MediaClock *masterClock;

private:

    int decodeVideo();

    int decodeFrameFromPacketQueue(AVFrame *pFrame);

    int decodeFrame(AVFrame *frame);

    int pushFrame(AVFrame *srcFrame, double pts, double duration, int64_t pos, int serial);

};


#endif
