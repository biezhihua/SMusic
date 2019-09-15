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

private:
    AVFormatContext *formatContext; // 解复用上下文

    FrameQueue *frameQueue;         // 帧队列

    int rotate;                     // 旋转角度

    Thread *decodeThread;           // 解码线程

    MediaClock *masterClock;        // 主时钟

private:
    // 解码视频帧
    int decodeVideo();

    int popFrame(AVFrame *pFrame);

    int decodeFrame(AVFrame *frame);

    int pushFrame(AVFrame *srcFrame, double pts, double duration, int64_t pos, int serial);

    double getFrameDuration(const AVRational &frame_rate) const;

    double getFramePts(const AVFrame *frame, const AVRational &time_base) const;
};


#endif //VIDEODECODER_H
