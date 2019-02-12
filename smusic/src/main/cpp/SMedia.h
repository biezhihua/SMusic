//
// Created by biezhihua on 2019/2/4.
//

#ifndef SMUSIC_S_AUDIO_H
#define SMUSIC_S_AUDIO_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};

#include "SPlayerStatus.h"
#include "SQueue.h"

class SMedia {

private:
    int streamIndex;
    AVCodec *pCodec = NULL;
    AVCodecParameters *pCodecParameters = NULL;
    SQueue *sQueue = NULL;
    AVCodecContext *pCodecContext = NULL;

public:

    SMedia(int streamIndex, AVCodec *pCodec, AVCodecParameters *pCodecParameters);

    ~SMedia();

    AVCodec *getCodec();

    AVCodecParameters *getCodecParameters();

    int getStreamIndex();

    int putAvPacketToQueue(AVPacket *pPacket);

    int getAvPacketFromQueue(AVPacket *pPacket);

    int getQueueSize();

    void setCodecContext(AVCodecContext *pCodecContext);

    AVCodecContext *getCodecContext();
};


#endif //SMUSIC_S_AUDIO_H
