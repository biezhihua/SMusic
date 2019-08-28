#ifndef SPLAYER_CORE_PLAY_H
#define SPLAYER_CORE_PLAY_H

class Surface;

class Audio;

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
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/pixdesc.h>
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

#define KEY_LOW_RESOLUTION "lowres"

#define KEY_REF_COUNTED_FRAMES "refcounted_frames"

#define KEY_THREADS "threads"

#define STREAM_TAG "Stream"

static const char *const OPT_RESAMPLE_SWR = "aresample_swr_opts";

static const char *const OPT_SAMPLE_FMTS = "sample_fmts";

static const char *const OPT_ALL_CHANNEL_COUNTS = "all_channel_counts";

static const char *const OPT_CHANNEL_LAYOUTS = "channel_layouts";

static const char *const OPT_CHANNEL_COUNTS = "channel_counts";

static const char *const OPT_SAMPLE_RATES = "sample_rates";

class Stream {

private:

    /// 刷新的包,用于在SEEK时，刷新数据队列
    AVPacket flushPacket;

    Audio *audio = nullptr;

    Surface *surface = nullptr;

    VideoState *videoState = nullptr;

    Options *options = nullptr;

    MessageQueue *msgQueue = nullptr;


public:

    Stream();

    ~Stream();

    int create();

    int destroy();

    void setMsgQueue(MessageQueue *msgQueue);

    void setAudio(Audio *audio);

    void setSurface(Surface *surface);

    void setOptions(Options *options);

    int togglePause();

    int forceRefresh();

    void stepToNextFrame();

    void streamSeek(int64_t pos, int64_t rel, int seekByBytes);

    int stop();

    int shutdown();

    int waitStop();

    int prepareStream(const char *fileName);

    int readThread();

    int videoThread();

    int subtitleThread();

    int audioThread();

    void streamsClose();

    VideoState *getVideoState() const;

    double getMasterClock();

    int getMasterSyncType();

    void checkExternalClockSpeed();

    double getFrameDuration(const Frame *current, const Frame *next);

    double getComputeTargetDelay(double duration);

    void updateVideoClockPts(double pts, int64_t pos, int serial);

    int streamTogglePause();

private:

    VideoState *streamsOpen();

    int getStartupVolume();

    int isRealTime(AVFormatContext *formatContext);

    int streamComponentOpen(int streamIndex);

    bool streamHasEnoughPackets(AVStream *stream, int index, PacketQueue *packetQueue);

    int streamComponentClose(AVStream *stream, int streamIndex);

    void closeReadThread(const VideoState *is, AVFormatContext *formatContext) const;

    int getVideoFrame(AVFrame *pFrame);

    int decoderDecodeFrame(Decoder *decoder, AVFrame *frame, AVSubtitle *subtitle);

    int queueFrameToFrameQueue(AVFrame *srcFrame, double pts, double duration, int64_t pos, int serial);

    bool isNoReadMore();

    bool isRetryPlay() const;

    int isPacketInPlayRange(const AVFormatContext *formatContext, const AVPacket *packet) const;

    const char *getForcedCodecName(int streamIndex, const AVCodecContext *codecContext) const;

    int64_t getValidChannelLayout(uint64_t channelLayout, int channels);

    int cmpAudioFormats(AVSampleFormat fmt1, int64_t channel_count1,
                        AVSampleFormat fmt2, int64_t channel_count2);

#if CONFIG_AVFILTER

    int configureAudioFilters(const char *audioFilters, int forceOutputFormat);

    int configureFilterGraph(AVFilterGraph *graph, const char *filterGraph,
                             AVFilterContext *srcFilterContext, AVFilterContext *sinkFilterContext);

    int configureVideoFilters(AVFilterGraph *filterGraph, VideoState *is, const char *filters, AVFrame *frame);

#endif
};

#endif //SPLAYER_CORE_PLAY_H
