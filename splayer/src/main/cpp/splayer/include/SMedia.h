#ifndef SPLAYER_S_AUDIO_H
#define SPLAYER_S_AUDIO_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};

#include "SStatus.h"
#include "SQueue.h"

class SMedia {

public:

    pthread_mutex_t mutex;

    double currentFrameTime;
    double currentTime;
    double currentRealTime;
    int64_t totalTime;
    double lastTimeMillis;
    double totalTimeMillis;
    double currentTimeMillis;
    int streamIndex;
    int sampleRate;
    double currentFramePTS;
    double delayRenderTime;
    double defaultDelayRenderTime = 0;

    AVRational timeBase;
    AVCodec *pCodec = NULL;
    AVCodecParameters *pCodecParameters = NULL;
    AVCodecContext *pCodecContext = NULL;

public:

    SMedia(int streamIndex, AVCodec *pCodec, AVCodecParameters *pCodecParameters);

    ~SMedia();

    int getSampleRate();

    void updateTime(AVFrame *pFrame, int dataSize);

    double getCurrentTime() const;

    double getCurrentRealTime() const;

    double getCurrentTimeMillis() const;

    double getTotalTimeMillis() const;

    bool isMinDiff();

    double getCurrentPTSByAVFrame(AVFrame *avFrame);

    double getFrameDiffTime(SMedia *audio, double pts);

    double getDelayRenderTime(double diff);

    void clearCodecContextBuffer();
};


#endif //SPLAYER_S_AUDIO_H
