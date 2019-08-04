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

    AVStream *videoStream;
    AVStream *audioStream;
    AVStream *subtitleStream;

    Mutex *continueReadThread;
    Mutex *accurateSeekMutex;
    Mutex *playMutex;

    Thread *readTid;

    AVFormatContext *ic;

    int abortRequest;
    int forceRefresh;
    int paused;
    int lastPaused;
    int queueAttachmentsReq;
    int seekReq;
    int seekFlags;
    int64_t seekPos;
    int64_t seekRel;
    int realTime;


    char *fileName;
    int yTop;
    int xLeft;
    int audioClockSerial;
    int audioVolume;
    int muted;

    int avSyncType;
    int pauseReq;

    int lastVideoStream;
    int lastAudioStream;
    int lastSubtitleStream;
    int videoStreamIndex;
    int audioStreamIndex;
    int subtitleStreamIndex;
    int eof;

    double maxFrameDuration;

    int showMode;

    int readPauseReturn;
};

#endif //SPLAYER_MAC_VIDEOSTATE_H
