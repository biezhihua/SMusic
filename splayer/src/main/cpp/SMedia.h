//
// Created by biezhihua on 2019/2/4.
//

#ifndef SMUSIC_S_AUDIO_H
#define SMUSIC_S_AUDIO_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};

#include "SStatus.h"
#include "SQueue.h"

class SMedia {

public:
    double currentFrameTime;
    double currentTime;
    int64_t totalTime;
    double lastTimeMillis;
    double totalTimeMillis;
    double currentTimeMillis;
    int streamIndex;
    int sampleRate;

    AVRational timeBase;
    AVCodec *pCodec = NULL;
    AVCodecParameters *pCodecParameters = NULL;
    AVCodecContext *pCodecContext = NULL;

public:

    SMedia(int streamIndex, AVCodec *pCodec, AVCodecParameters *pCodecParameters);

    ~SMedia();

    int getSampleRate();

    void updateTime(AVFrame *pFrame, int dataSize);

    double getCurrentTimeMillis() const;

    double getTotalTimeMillis() const;

    bool isMinDiff();
};


#endif //SMUSIC_S_AUDIO_H
