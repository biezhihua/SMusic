
#include <Decoder.h>

#include "Decoder.h"

void Decoder::decoderInit(AVCodecContext *codecContext, PacketQueue *packetQueue, Mutex *emptyQueueMutex) {
    avctx = codecContext;
    packetQueue = packetQueue;
    emptyQueueCond = emptyQueueMutex;
    startPts = AV_NOPTS_VALUE;
    pktSerial = -1;
}

void Decoder::decoderDestroy() {
    av_packet_unref(&pkt);
    avcodec_free_context(&avctx);
}

void Decoder::decoderAbort(FrameQueue *frameQueue) {
    if (packetQueue) {
        packetQueue->packetQueueAbort();
    }
    if (frameQueue) {
        frameQueue->frameQueueSignal();
    }
    if (decoderTid) {
        decoderTid->waitThread();
        decoderTid = nullptr;
    }
    if (packetQueue) {
        packetQueue->packetQueueFlush();
    }
}

int Decoder::decoderStart(int (*fn)(void *), void *arg) {
    if (packetQueue) {
        packetQueue->packetQueueStart();
    }
    decoderTid = new Thread(fn, arg, "decoder");
    if (decoderTid) {
        ALOGD("%s create decoder thread fail ", __func__);
        return NEGATIVE(S_NOT_MEMORY);
    }
    return POSITIVE;
}

