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
    AVPacket *pDecodePacket = NULL;
    AVPacket *pResamplePacket = NULL;
    AVFrame *pResampleFrame = NULL;
    uint8_t *pBuffer = NULL;
    string *pSource = NULL;

public:

    SFFmpeg();

    ~SFFmpeg();

    void setSource(string *pSource);

    int decodeMediaInfo();

    int decodeAudioFrame();

    int resampleAudio();

    SMedia *getAudio();

    SMedia *getVideo();
};


#endif //SMUSIC_S_FFMPEG_H
