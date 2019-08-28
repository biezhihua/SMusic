#ifndef SPLAYER_CORE_DECODER_H
#define SPLAYER_CORE_DECODER_H


#include "Thread.h"
#include "Mutex.h"
#include "PacketQueue.h"
#include "FrameQueue.h"

extern "C" {
#include <libavutil/time.h>
#include <libavutil/rational.h>
#include <libavutil/mem.h>
#include <libavutil/log.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/avutil.h>
#include <libavutil/dict.h>
#include <libavutil/avstring.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include "CmdUtils.h"
};

class Decoder {
public:

    /// 解码上下文
    AVCodecContext *codecContext;

    /// 包队列
    PacketQueue *packetQueue;

    /// 包数据
    AVPacket packet;

    /// 包序列，作seek时使用，作为区分前后帧序列
    int packetSeekSerial;

    /// 是否已结束
    int finished;

    /// 是否有包再等待
    int packetPending;

    /// 空队列条件变量
    Mutex *emptyQueueCond;

    /// 开始的间戳
    int64_t startPts;

    /// 开始的额外参数
    AVRational startPtsTb;

    /// 下一帧时间戳
    int64_t nextPts;

    /// 下一阵额外参数
    AVRational nextPtsTb;

    /// 解码线程
    Thread *decoderTid;

public:
    void init(AVCodecContext *codecContext, PacketQueue *packetQueue, Mutex *emptyQueueMutex);

    void destroy();

    void abort(FrameQueue *frameQueue);

    int start(const char *name, int (*fn)(void *), void *arg);
};


#endif //SPLAYER_CORE_DECODER_H
