#ifndef SPLAYER_PLAY_H
#define SPLAYER_PLAY_H

#include "Log.h"
#include "Define.h"
#include "Mutex.h"
#include "MessageQueue.h"
#include "AOut.h"
#include "VOut.h"
#include "Pipeline.h"
#include "State.h"
#include "Error.h"
#include "VideoState.h"
#include "Thread.h"

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

static const char *const SCAN_ALL_PMTS = "scan_all_pmts";

static const char *const OGG = "ogg";

static const char *const TITLE = "title";

static const char *const FORMAT_RTP = "rtp";

static const char *const FORMAT_RTSP = "rtsp";

static const char *const FORMAT_SDP = "sdp";

static const char *const URL_FORMAT_RTP = "rtp:";

static const char *const URL_FORMAT_UDP = "udp:";

class FFPlay {

private:

    /**
     * Self Implement
     */
    MessageQueue *msgQueue = nullptr;
    AOut *aOut = nullptr;
    VOut *vOut = nullptr;
    Pipeline *pipeline = nullptr;

    /**
     *  Current Context
     */
    VideoState *videoState = nullptr;
    AVPacket flushPacket;
    int isFullScreen;

    /**
     * Format/Codec Options
     */
    AVDictionary *optionFormat = nullptr;
    AVDictionary *optionCodec = nullptr;
    AVDictionary *optionSws = nullptr;
    AVDictionary *optionSwr = nullptr;
    AVDictionary *optionSwrPreset = nullptr;
    AVDictionary *optionPlayer = nullptr;

    /**
     * Options specified by the user
     */
    AVInputFormat *optionInputFormat = nullptr;
    char *optionInputFileName = nullptr;
    char *optionWindowTitle = nullptr;
    int optionDefaultWidth = 640;
    int optionDefaultHeight = 480;
    int optionScreenWidth = 0;
    int optionScreenHeight = 0;
    int optionScreenLeft = 0;
    int optionScreenTop = 0;
    char *optionWantedStreamSpec[AVMEDIA_TYPE_NB] = {nullptr};
    int optionSeekByBytes = -1;
    float optionSeekInterval = 10;
    int optionAudioDisable = 0;
    int optionVideoDisable = 0;
    int optionSubtitleDisable = 0;
    int optionStartupVolume = 100;
    int optionSyncType = SYNC_TYPE_AUDIO_MASTER;
    int64_t optionStartTime = AV_NOPTS_VALUE;
    int64_t optionDuration = AV_NOPTS_VALUE;
    int optionFast = 0;
    int optionGenpts = 0;
    int optionLowres = 0;
    int optionDecoderReorderPts = -1;
    int optionAutoExit;
    int optionLoop = 1;
    int optionFrameDrop = -1;
    int optionShowMode = SHOW_MODE_NONE;
    char *optionInputFormatName = nullptr;
    char *optionAudioCodecName = nullptr;
    char *optionSubtitleCodecName = nullptr;
    char *optionVideoCodecName = nullptr;
    double optionRDFTSpeed = 0.02;
    int optionInfiniteBuffer = -1;
    int optionShowStatus = 1;
    int optionFindStreamInfo = 1;

public:
    FFPlay();

    ~FFPlay();

    void setAOut(AOut *aOut);

    void setVOut(VOut *vOut);

    void setPipeline(Pipeline *pipeline);

    MessageQueue *getMsgQueue() const;

    int stop();

    int shutdown();

    int waitStop();

    int prepareAsync(const char *fileName);

    int getMsg(Message *pMessage, bool block);

    /**
     * this thread gets the stream from the disk or the network
     */
    int readThread();

    int videoThread();

    int subtitleThread();

    int audioThread();

    int refreshThread();

private:
    void showVersionsAndOptions();

    void showDict(const char *tag, AVDictionary *dict);

    VideoState *streamOpen();

    void streamClose();

    int getStartupVolume();

    int isRealTime(AVFormatContext *pContext);

    int streamComponentOpen(int streamIndex);

    int streamHasEnoughPackets(AVStream *stream, int index, PacketQueue *packetQueue);

    int streamComponentClose(AVStream *stream, int streamIndex);

    double getMasterClock();

    int getMasterSyncType();

    void closeReadThread(const VideoState *is, AVFormatContext *&formatContext) const;

    void stepToNextFrame();

    void streamSeek(int64_t pos, int64_t rel, int seek_by_bytes);

    int getVideoFrame(AVFrame *pFrame);

    int decoderDecodeFrame(Decoder *decoder, AVFrame *frame, AVSubtitle *subtitle);

    int queuePicture(AVFrame *srcFrame, double pts, double duration, int64_t pos, int serial);

    void videoRefresh(double *remainingTime);

    void checkExternalClockSpeed();

    void videoDisplay();

    double frameDuration(Frame *vp, Frame *nextvp);

    double computeTargetDelay(double delay);

    void updateVideoPts(double pts, int64_t pos, int serial);

    void syncClockToSlave(Clock *c, Clock *slave);

    int videoOpen();

    void videoImageDisplay();

    int uploadTexture(AVFrame *pFrame);
};

#endif //SPLAYER_PLAY_H
