#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
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

    Mutex *avMutex = nullptr;
    Mutex *vfMutex = nullptr;

    /**
     * Message Loop
     */
    MessageQueue *msgQueue = nullptr;

    /**
     * Audio Output
     */
    AOut *aOut = nullptr;

    /**
     * Video Output
     */
    VOut *vOut = nullptr;

    /**
     * Data input
     */
    Pipeline *pipeline = nullptr;

    /**
     * format/codec options
     */
    AVDictionary *formatOpts = nullptr;
    AVDictionary *codecOpts = nullptr;
    AVDictionary *swsDict = nullptr;
    AVDictionary *swrOpts = nullptr;
    AVDictionary *swrPresetOpts = nullptr;
    AVDictionary *playerOpts = nullptr;

    /**
     * Options specified by the user
     */

    AVInputFormat *inputFormat = nullptr;
    char *inputFileName = nullptr;

    VideoState *videoState = nullptr;

    AVPacket flushPacket;

    int videoQueueSize;

    int startupVolume = 100;

    int avSyncType = AV_SYNC_AUDIO_MASTER;

    char *inputFormatName;

    char *wantedStreamSpec[AVMEDIA_TYPE_NB] = {0};

    char *audioCodecName;

    char *subtitleCodecName;

    char *videoCodecName;

    int genpts;

    int findStreamInfo;

    int seekByBytes;

    char *windowTitle;

    int64_t startTime = AV_NOPTS_VALUE;

    int64_t duration = AV_NOPTS_VALUE;

    int audioDisable = 0;

    int videoDisable = 0;

    int subtitleDisable = 0;

    int showMode = SHOW_MODE_NONE;

    int infiniteBuffer = -1;

    int renderWaitStart;

    int startOnPrepared;

    bool prepared;

    int autoResume;

    int seekAtStart;

    int loop = 1;

    int autoExit;

    int showStatus = 1;

    int lowres = 0;

    int fast = 0;

    int frameDrop = -1;

    int decoderReorderPts = -1;

    double rdftspeed = 0.02;

    int defaultWidth = 640;
    int defaultHeight = 480;
    int screenWidth = 0;
    int screenHeight = 0;
    int screen_left = 0;
    int screen_top = 0;
    int is_full_screen;


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

#pragma clang diagnostic pop