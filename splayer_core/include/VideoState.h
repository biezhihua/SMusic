#ifndef SPLAYER_MAC_VIDEOSTATE_H
#define SPLAYER_MAC_VIDEOSTATE_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
};

#include "FrameQueue.h"
#include "Decoder.h"
#include "PacketQueue.h"
#include "Clock.h"
#include "Thread.h"
#include "AudioParams.h"

class VideoState {

public:
    /// Common

    AVInputFormat *inputFormat;
    AVFormatContext *formatContext;
    Thread *readThread;
    Mutex *continueReadThread;
    Clock exitClock;

    int showMode = SHOW_MODE_VIDEO;

    SwsContext *imgConvertCtx;
    SwsContext *subConvertCtx;

    // seek
    int seekReq;
    int seekFlags;
    int64_t seekPos;
    int64_t seekRel;

    double maxFrameDuration;
    int abortRequest;
    int forceRefresh;
    int paused;
    int lastPaused;
    // https://segmentfault.com/a/1190000018373504?utm_source=tag-newest
    int queueAttachmentsReq;
    int realTime;
    char *fileName;
    int width;
    int height;
    int yTop;
    int xLeft;
    int syncType;
    int eof;
    int readPauseReturn;
    RDFTContext *rdft;
    int rdft_bits;
    FFTSample *rdft_data;
    int frameDropsEarly;
    int frameDropsLate;
    double lastVisTime;
    double frameTimer;
    double frameLastFilterDelay;
    double frameLastReturnedTime;
    int step;

#if CONFIG_AVFILTER
    int vfilterIdx;
    AVFilterGraph *agraph;              // audio filter graph
#endif

    /// Subtitle
    FrameQueue subtitleFrameQueue;
    PacketQueue subtitlePacketQueue;
    AVStream *subtitleStream;
    Decoder subtitleDecoder;
    int subtitleLastStreamIndex;
    int subtitleStreamIndex;

    /// Video
    AVStream *videoStream;
    FrameQueue videoFrameQueue;
    Decoder videoDecoder;
    PacketQueue videoPacketQueue;
    Clock videoClock;
    int videoStreamIndex;
    int videoLastStreamIndex;

#if CONFIG_AVFILTER
    AVFilterContext *inVideoFilter;   // the first filter in the video chain
    AVFilterContext *outVideoFilter;  // the last filter in the video chain
#endif

    /// Audio
    AVStream *audioStream;
    FrameQueue audioFrameQueue;
    PacketQueue audioPacketQueue;
    Decoder audioDecoder;
    Clock audioClock;
    int audioLastStreamIndex;
    int audioStreamIndex;
    int audioClockSerial;
    int audioVolume;
    int audioMuted;

    SwrContext *audioSwrContext;
    uint8_t *audioBuf;
    uint8_t *audioBuf1;
    int audioHwBufSize;
    int audioBufIndex; /* in bytes */
    unsigned int audioBufSize; /* in bytes */
    unsigned int audioBuf1Size;
    double audio_diff_cum; /* used for AV difference average computation */
    double audioDiffAvgCoef;
    double audioDiffThreshold;
    int audioDiffAvgCount;

    AudioParams audioSrc;
#if CONFIG_AVFILTER
    AudioParams audioFilterSrc;
#endif
    AudioParams audioTarget;

#if CONFIG_AVFILTER
    AVFilterContext *inAudioFilter;   // the first filter in the audio chain
    AVFilterContext *outAudioFilter;  // the last filter in the audio chain
#endif
};

#endif //SPLAYER_MAC_VIDEOSTATE_H
