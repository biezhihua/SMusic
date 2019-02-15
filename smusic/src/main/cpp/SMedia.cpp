//
// Created by biezhihua on 2019/2/4.
//

#include "SMedia.h"

SMedia::SMedia(int streamIndex, AVCodec *pCodec, AVCodecParameters *pCodecParameters) {
    this->streamIndex = streamIndex;
    this->pCodec = pCodec;
    this->pCodecParameters = pCodecParameters;

}

SMedia::~SMedia() {
    streamIndex = -1;
    pCodec = NULL;
    pCodecParameters = NULL;
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

void SMedia::setCodecContext(AVCodecContext *pCodecContext) {
    this->pCodecContext = pCodecContext;
}

AVCodecContext *SMedia::getCodecContext() {
    return this->pCodecContext;
}

int SMedia::getSampleRate() {
    if (pCodecParameters != NULL) {
        return pCodecParameters->sample_rate;
    }
    return 0;
}

