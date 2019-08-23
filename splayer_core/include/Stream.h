#ifndef SPLAYER_PLAY_H
#define SPLAYER_PLAY_H

class Surface;

class Audio;

class Stream;

#include "Log.h"
#include "Define.h"
#include "Mutex.h"
#include "MessageQueue.h"
#include "Audio.h"
#include "Surface.h"
#include "Stream.h"
#include "State.h"
#include "Error.h"
#include "VideoState.h"
#include "Thread.h"
#include "Options.h"

extern "C" {
#include <libavutil/time.h>
#include <libavutil/rational.h>
#include <libavutil/mem.h>
#include <libavutil/log.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/avutil.h>
#include <libavutil/dict.h>
#include <libavutil/avstring.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include "CmdUtils.h"
};

#define SCAN_ALL_PMTS  "scan_all_pmts"

#define FORMAT_OGG  "ogg"

#define TITLE  "title"

#define FORMAT_RTP  "rtp"

#define FORMAT_RTSP  "rtsp"

#define FORMAT_SDP  "sdp"

#define URL_FORMAT_RTP  "rtp:"

#define URL_FORMAT_UDP  "udp:"

#define FFPLAY_TAG "FFPlay"

class Stream {

private:
    Audio *audio = nullptr;
    Surface *surface = nullptr;
    VideoState *videoState = nullptr;
    AVPacket flushPacket;
    Options *options = nullptr;
    MessageQueue *msgQueue = nullptr;

public:

    virtual int create();

    virtual int destroy();

    void setMsgQueue(MessageQueue *msgQueue);

    double remainingTime = 0.0f;

    void togglePause();

    void forceRefresh();

    void setupToNextFrame();

    void streamSeek(int64_t pos, int64_t rel, int seekByBytes);

public:
    Stream();

    ~Stream();

    void setAudio(Audio *audio);

    void setSurface(Surface *surface);

    int stop();

    int shutdown();

    int waitStop();

    int prepareStream(const char *fileName);

    /**
     * this thread gets the stream from the disk or the network
     */
    int readThread();

    int videoThread();

    int subtitleThread();

    int audioThread();

    VideoState *getVideoState() const;

    int refresh();

    void setOptions(Options *options);

    void streamClose();

    double getMasterClock();

private:

    VideoState *streamOpen();

    int getStartupVolume();

    int isRealTime(AVFormatContext *pContext);

    int streamComponentOpen(int streamIndex);

    int streamHasEnoughPackets(AVStream *stream, int index, PacketQueue *packetQueue);

    int streamComponentClose(AVStream *stream, int streamIndex);

    int getMasterSyncType();

    void closeReadThread(const VideoState *is, AVFormatContext *&formatContext) const;

    void stepToNextFrame();

    int getVideoFrame(AVFrame *pFrame);

    int decoderDecodeFrame(Decoder *decoder, AVFrame *frame, AVSubtitle *subtitle);

    int queueFrameToFrameQueue(AVFrame *srcFrame, double pts, double duration, int64_t pos, int serial);

    void refreshVideo(double *remainingTime);

    void checkExternalClockSpeed();

    void displayVideo();

    double getFrameDuration(Frame *current, Frame *next);

    double getComputeTargetDelay(double duration);

    void updateVideoClockPts(double pts, int64_t pos, int serial);

    void syncClockToSlave(Clock *c, Clock *slave);

    int displayWindow();

    void displayVideoImage();

    void displayVideoAudio();

    bool isNoReadMore();

    bool isRetryPlay() const;

    int isPacketInPlayRange(const AVFormatContext *formatContext, const AVPacket *packet) const;

    void streamTogglePause();
};

#endif //SPLAYER_PLAY_H
