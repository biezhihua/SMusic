//
// Created by biezhihua on 2019-07-16.
//

#ifndef ANDROID_SPLAYER_DECODER_H
#define ANDROID_SPLAYER_DECODER_H
extern "C" {
#include <libavutil/time.h>
#include <libavutil/rational.h>
#include <libavutil/mem.h>
#include <libavutil/log.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
};

#include "Thread.h"
#include "Mutex.h"
#include "FFPlay.h"
#include "MyAVPacketList.h"

class Decoder {
public:
    AVPacket pkt;
    PacketQueue *queue;
    AVCodecContext *avctx;
    int pkt_serial;
    int finished;
    int packet_pending;
    Mutex *empty_queue_cond;
    int64_t start_pts;
    AVRational start_pts_tb;
    int64_t next_pts;
    AVRational next_pts_tb;
    Thread *decoder_tid;
};


#endif //ANDROID_SPLAYER_DECODER_H
