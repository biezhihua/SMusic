#ifndef SPLAYER_MAC_VIDEOSTATE_H
#define SPLAYER_MAC_VIDEOSTATE_H

extern "C" {
#include <libavformat/avformat.h>
};

#include "FrameQueue.h"
#include "Decoder.h"
#include "PacketQueue.h"

class VideoState {

public:
    AVInputFormat *inputFormat;

    FrameQueue videoFQueue;
    FrameQueue audioFQueue;
    FrameQueue subtitleFQueue;

    PacketQueue videoPQueue;
    PacketQueue audioPQueue;
    PacketQueue subtitlePQueue;

    Decoder videoDecoder;
    Decoder audioDecoder;
    Decoder subtitleDecoder;

    char *fileName;
    int yTop;
    int xLeft;
};

#endif //SPLAYER_MAC_VIDEOSTATE_H
