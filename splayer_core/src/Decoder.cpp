#include "Decoder.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

void Decoder::decoderInit(AVCodecContext *codecContext, PacketQueue *packetQueue, Mutex *emptyQueueMutex) {
    Decoder::codecContext = codecContext;
    Decoder::packetQueue = packetQueue;
    Decoder::emptyQueueCond = emptyQueueMutex;
    Decoder::startPts = AV_NOPTS_VALUE;
    Decoder::packetSerial = -1;
}

void Decoder::decoderDestroy() {
    av_packet_unref(&packet);
    avcodec_free_context(&codecContext);
}

void Decoder::decoderAbort(FrameQueue *frameQueue) {
    if (packetQueue) {
        packetQueue->abort();
    }
    if (frameQueue) {
        frameQueue->signal();
    }
    if (decoderTid) {
        decoderTid->waitThread();
        decoderTid = nullptr;
    }
    if (packetQueue) {
        packetQueue->flush();
    }
}

int Decoder::decoderStart(const char *name, int (*fn)(void *), void *arg) {
    if (packetQueue) {
        packetQueue->start();
    }
    decoderTid = new Thread(fn, arg, name);
    if (!decoderTid) {
        ALOGD("%s create decoder thread fail ", __func__);
        return NEGATIVE(S_NO_MEMORY);
    }
    return POSITIVE;
}


#pragma clang diagnostic pop