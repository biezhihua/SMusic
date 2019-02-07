//
// Created by biezhihua on 2019/2/4.
//

#ifndef SMUSIC_S_AUDIO_H
#define SMUSIC_S_AUDIO_H

#include "../../../../ffmpeg-output/ffmpeg-x86/include/libavcodec/avcodec.h"
#include "SQueue.h"

class SMedia {

private:
    int streamIndex;
    AVCodec *pCodec = NULL;
    AVCodecParameters *pCodecParameters = NULL;
    SQueue *sQueue = NULL;

public:

    SMedia(int streamIndex, AVCodec *pCodec, AVCodecParameters *pCodecParameters);

    ~SMedia();

    AVCodec *getCodec();

    AVCodecParameters *getCodecParameters();

    int getStreamIndex();

    int putAvPacketToQueue(AVPacket *pPacket);

    int getAvPacketFromQueue(AVPacket *pPacket);

    int getQueueSize();
};


#endif //SMUSIC_S_AUDIO_H
