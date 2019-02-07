//
// Created by biezhihua on 2019/2/4.
//

#ifndef SMUSIC_S_FFMPEG_H
#define SMUSIC_S_FFMPEG_H


extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};

#include <string>
#include "SMedia.h"
#include "SJavaMethods.h"

class SFFmpeg {
private:
    AVFormatContext *pFormatContext = NULL;
    SMedia *pAudio = NULL;
    SMedia *pVideo = NULL;

public:
    string *pSource;

public:

    SFFmpeg();

    ~SFFmpeg();

    void setSource(string *pSource);

    int decodeMediaInfo();

    int decodeAudioFrame();
};


#endif //SMUSIC_S_FFMPEG_H
