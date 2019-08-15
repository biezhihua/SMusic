#ifndef SPLAYER_MAC_DECODER_H
#define SPLAYER_MAC_DECODER_H


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
    AVCodecContext *codecContext;
    PacketQueue *packetQueue;
    AVPacket packet;
    int packetSerial;
    int finished;
    int packetPending;
    Mutex *emptyQueueCond;
    int64_t startPts;
    AVRational startPtsTb;
    int64_t nextPts;
    AVRational nextPtsTb;
    Thread *decoderTid;

public:
    void decoderInit(AVCodecContext *codecContext, PacketQueue *packetQueue, Mutex *emptyQueueMutex);

    void decoderDestroy();

    void decoderAbort(FrameQueue *frameQueue);

    int decoderStart(const char *name, int (*fn)(void *), void *arg);
};


#endif //SPLAYER_MAC_DECODER_H
