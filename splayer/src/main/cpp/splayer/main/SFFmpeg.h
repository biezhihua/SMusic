#ifndef SPLAYER_S_FFMPEG_H
#define SPLAYER_S_FFMPEG_H


static const int CACHE_SIZE = 40;

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
};

#include <string>
#include <stdio.h>
#include "SMedia.h"
#include "SJavaMethods.h"
#include "SError.h"

class SFFmpeg {
private:
    string *pSource = nullptr;

    SJavaMethods *pJavaMethods = nullptr;

    AVFormatContext *pFormatContext = nullptr;

    AVPacket *pDecodeRawFramePacket = nullptr;

    SMedia *pAudioMedia = nullptr;
    SQueue *pAudioQueue = nullptr;

    SMedia *pVideoMedia = nullptr;
    SQueue *pVideoQueue = nullptr;

    AVPacket *pAudioPacket = nullptr;
    AVFrame *pAudioFrame = nullptr;

    AVPacket *pVideoPacket = nullptr;
    AVFrame *pVideoFrame = nullptr;

    uint8_t *pBuffer = nullptr;

    SStatus *pStatus = nullptr;

    pthread_mutex_t seekMutex;

    bool isLoading = false;

    int channelSampleNumbers = 0;

    const AVBitStreamFilter *pBSFilter = nullptr;


public:

    SFFmpeg(SStatus *pStatus, SJavaMethods *pJavaMethods);

    ~SFFmpeg();

    void setSource(string *pSource);

    int decodeMediaInfo();

    int decodeFrame();

    int softDecodeVideo();

    int mediaDecodeVideo();

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

    void startSoftDecode();

    void startMediaDecode();
};


#endif //SPLAYER_S_FFMPEG_H
