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

int SMedia::getSampleRate() {
    if (pCodecParameters != NULL) {
        sampleRate = pCodecParameters->sample_rate;
    }
    return sampleRate;
}

void SMedia::updateTime(AVFrame *pFrame, int dataSize) {
    currentFrameTime = (pFrame->pts * av_q2d(timeBase));
    if (currentFrameTime < currentTime) {
        currentFrameTime = currentTime;
    }
    currentTime = currentFrameTime;
    currentTime += dataSize / ((double) (getSampleRate() * 2 * 2));
    currentTimeMillis = (currentTime * 1000);
}

double SMedia::getCurrentTimeMillis() const {
    return currentTimeMillis;
}

double SMedia::getTotalTimeMillis() const {
    return totalTimeMillis;
}

bool SMedia::isMinDiff() {
    if (currentTimeMillis - lastTimeMillis >= 1000) {
        lastTimeMillis = currentTimeMillis;
        return true;
    }
    return false;
}

