//
// Created by biezhihua on 2019/2/4.
//

#include "SMedia.h"

SMedia::SMedia(int streamIndex, AVCodec *pCodec, AVCodecParameters *pCodecParameters) {
    this->streamIndex = streamIndex;
    this->pCodec = pCodec;
    this->pCodecParameters = pCodecParameters;
    this->sQueue = new SQueue();
}

SMedia::~SMedia() {
    streamIndex = -1;
    pCodec = NULL;
    pCodecParameters = NULL;
    delete sQueue;
    sQueue = NULL;
}

AVCodec *SMedia::getCodec() {
    return pCodec;
}

AVCodecParameters *SMedia::getCodecParameters() {
    return pCodecParameters;
}

int SMedia::getStreamIndex() {
    return streamIndex;
}

int SMedia::putAvPacketToQueue(AVPacket *pPacket) {
    if (sQueue != NULL) {
        return sQueue->putAvPacket(pPacket);
    }
    return 0;
}

int SMedia::getAvPacketFromQueue(AVPacket *pPacket) {
    if (sQueue != NULL) {
        return sQueue->getAvPacket(pPacket);
    }
    return 0;
}

int SMedia::getQueueSize() {
    if (sQueue != NULL) {
        return sQueue->getSize();
    }
    return 0;
}

