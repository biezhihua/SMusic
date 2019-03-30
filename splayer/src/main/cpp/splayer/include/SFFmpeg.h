#ifndef SPLAYER_S_FFMPEG_H
#define SPLAYER_S_FFMPEG_H


extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
};

#include <string>
#include <stdio.h>
#include "SMedia.h"
#include "SJavaMethods.h"
#include "SError.h"

class SFFmpeg {
private:
    string *pSource = NULL;

    SJavaMethods *pJavaMethods = NULL;

    AVFormatContext *pFormatContext = NULL;

    SMedia *pAudio = NULL;
    SQueue *pAudioQueue = NULL;

    SMedia *pVideo = NULL;
    SQueue *pVideoQueue = NULL;

    AVPacket *pDecodePacket = NULL;

    AVPacket *pAudioPacket = NULL;
    AVFrame *pAudioFrame = NULL;

    AVPacket *pVideoPacket = NULL;
    AVFrame *pVideoFrame = NULL;

    uint8_t *pBuffer = NULL;

    SStatus *pStatus = NULL;

    pthread_mutex_t seekMutex;

    int64_t seekTargetMillis = 0;

    double seekStartMillis = 0;

    bool isLoading = false;

    int channelSampleNumbers = 0;

public:

    SFFmpeg(SStatus *pStatus, SJavaMethods *pJavaMethods);

    ~SFFmpeg();

    void setSource(string *pSource);

    int decodeMediaInfo();

    int decodeFrame();

    int decodeVideo();

    int decodeAudio();

    SMedia *getAudioMedia();

    SMedia *getVideoMedia();

    double getTotalTimeMillis();

    double getCurrentTimeMillis();

    void releasePacket(AVPacket **avPacket);

    void releaseFrame(AVFrame **avFrame);

    SQueue *getAudioQueue();

    SQueue *getVideoQueue();

    int getAudioAvPacketFromAudioQueue(AVPacket *pPacket);

    int getVideoAvPacketFromAudioQueue(AVPacket *pPacket);

    uint8_t *getBuffer();

    int stop();

    int release();

    void seek(int64_t millis);

    int getChannelSampleNumbers() const;

    void sleep();
};


#endif //SPLAYER_S_FFMPEG_H
