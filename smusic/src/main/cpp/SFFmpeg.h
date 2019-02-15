//
// Created by biezhihua on 2019/2/4.
//

#ifndef SMUSIC_S_FFMPEG_H
#define SMUSIC_S_FFMPEG_H


extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
};

#include <string>
#include "SMedia.h"
#include "SJavaMethods.h"
#include "SError.h"

class SFFmpeg {
private:
    AVFormatContext *pFormatContext = NULL;
    SMedia *pAudio = NULL;
    SMedia *pVideo = NULL;
    SQueue *pAudioQueue = NULL;
    SQueue *pVideoQueue = NULL;
    AVPacket *pDecodePacket = NULL;
    AVPacket *pResamplePacket = NULL;
    AVFrame *pResampleFrame = NULL;
    uint8_t *pBuffer = NULL;
    string *pSource = NULL;
    SStatus *pStatus = NULL;

public:

    SFFmpeg(SStatus *pStatus);

    ~SFFmpeg();

    void setSource(string *pSource);

    int decodeMediaInfo();

    int decodeAudioFrame();

    int resampleAudio();

    SMedia *getAudio();

    SMedia *getVideo();

    void releasePacket();

    void releaseFrame();

    SQueue *getAudioQueue();

    SQueue *getVideoQueue();

    int getAvPacketFromQueue(AVPacket *pPacket);

    uint8_t *getBuffer();

    void reset();
};


#endif //SMUSIC_S_FFMPEG_H
