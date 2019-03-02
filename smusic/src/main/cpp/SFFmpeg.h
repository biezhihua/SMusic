//
// Created by biezhihua on 2019/2/4.
//

#ifndef SMUSIC_S_FFMPEG_H
#define SMUSIC_S_FFMPEG_H


extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
};

#include <string>
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
    AVPacket *pResamplePacket = NULL;

    AVFrame *pResampleFrame = NULL;

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

    int decodeAudioFrame();

    int resampleAudio();

    SMedia *getAudio();

    SMedia *getVideo();

    double getTotalTimeMillis();

    double getCurrentTimeMillis();

    void releasePacket();

    void releaseFrame();

    SQueue *getAudioQueue();

    SQueue *getVideoQueue();

    int getAvPacketFromQueue(AVPacket *pPacket);

    uint8_t *getBuffer();

    int stop();

    int release();

    void seek(int64_t millis);

    int getChannelSampleNumbers() const;

    void sleep();
};


#endif //SMUSIC_S_FFMPEG_H
