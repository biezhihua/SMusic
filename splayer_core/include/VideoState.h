#ifndef SPLAYER_MAC_VIDEOSTATE_H
#define SPLAYER_MAC_VIDEOSTATE_H

extern "C" {
#include <libavformat/avformat.h>
};

#include "FrameQueue.h"
#include "Decoder.h"
#include "PacketQueue.h"
#include "Clock.h"
#include "Thread.h"

class VideoState {

public:
    AVInputFormat *inputFormat;

    FrameQueue videoFrameQueue;
    FrameQueue audioFrameQueue;
    FrameQueue subtitleFrameQueue;

    PacketQueue videoPacketQueue;
    PacketQueue audioPacketQueue;
    PacketQueue subtitlePacketQueue;

    Decoder videoDecoder;
    Decoder audioDecoder;
    Decoder subtitleDecoder;

    Clock videoClock;
    Clock audioClock;
    Clock subtitleClock;

    Mutex *continueReadThread;
    Mutex *accurateSeekMutex;
    Mutex *playMutex;

    Thread *readTid;

    char *fileName;
    int yTop;
    int xLeft;
    int audioClockSerial;
    int audioVolume;
    int muted;

    int avSyncType;
    int pauseReq;

};

#endif //SPLAYER_MAC_VIDEOSTATE_H
