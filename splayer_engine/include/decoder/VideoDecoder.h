#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include <decoder/MediaDecoder.h>
#include <player/PlayerState.h>
#include <sync/MediaClock.h>

class VideoDecoder : public MediaDecoder {
    const char *const TAG = "VideoDecoder";
public:
    VideoDecoder(AVFormatContext *pFormatCtx, AVCodecContext *avctx, AVStream *stream, int streamIndex,
                 PlayerState *playerState, AVPacket *flushPacket, Condition *pCondition);

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

    int popFrame(AVFrame *pFrame);

    int decodeFrame(AVFrame *frame);

    int pushFrame(AVFrame *srcFrame, double pts, double duration, int64_t pos, int serial);

    double getFrameDuration(const AVRational &frame_rate) const;

    double getFramePts(const AVFrame *frame, const AVRational &time_base) const;
};


#endif //VIDEODECODER_H
