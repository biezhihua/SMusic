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
    Clock exitClock;

    AVStream *videoStream;
    AVStream *audioStream;
    AVStream *subtitleStream;

    Mutex *continueReadThread;

    Thread *readThread;

    AVFormatContext *formatContext;

    SwsContext *imgConvertCtx;
    SwsContext *subConvertCtx;

    int abortRequest;

    int forceRefresh;

    int paused;
    
    int lastPaused;

    // https://segmentfault.com/a/1190000018373504?utm_source=tag-newest
    int queueAttachmentsReq;

    int realTime;

    char *fileName;
    int width, height;
    int yTop;
    int xLeft;
    int audioClockSerial;
    int audioVolume;
    int muted;

    int syncType;
    int pauseReq;

    int lastVideoStreamIndex;
    int lastAudioStreamIndex;
    int lastSubtitleStreamIndex;
    int videoStreamIndex;
    int audioStreamIndex;
    int subtitleStreamIndex;
    int eof;

    double maxFrameDuration;

    int showMode = SHOW_MODE_VIDEO;

    int readPauseReturn;

    struct SwrContext *swrContext;
    uint8_t *audioBuf;
    uint8_t *audioBuf1;
    unsigned int audio_buf_size; /* in bytes */
    unsigned int audioBuf1Size;
    RDFTContext *rdft;
    int rdft_bits;
    FFTSample *rdft_data;

    double frameLastFilterDelay;

    int frameDropsEarly;
    int frameDropsLate;

    // seek
    int seekReq;
    int seekFlags;
    int64_t seekPos;
    int64_t seekRel;

    double lastVisTime;

    double frameTimer;

    int step;

};

#endif //SPLAYER_MAC_VIDEOSTATE_H
