#ifndef SPLAYER_PLAY_H
#define SPLAYER_PLAY_H

class Surface;

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

#define OGG  "ogg"

#define TITLE  "title"

#define FORMAT_RTP  "rtp"

#define FORMAT_RTSP  "rtsp"

#define FORMAT_SDP  "sdp"

#define URL_FORMAT_RTP  "rtp:"

#define URL_FORMAT_UDP  "udp:"

#define FFPLAY_TAG "FFPlay"

class FFPlay {

private:

    /**
     * Self Implement
     */
    MessageQueue *msgQueue = nullptr;
    Audio *audio = nullptr;
    Surface *surface = nullptr;
    Stream *stream = nullptr;

    /**
     *  Current Context
     */
    VideoState *videoState = nullptr;
    AVPacket flushPacket;

public:

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
    AVInputFormat *optionInputFormat = nullptr; // force format
    char *optionInputFileName = nullptr;
    char *optionWindowTitle = nullptr; // set window title
    int optionIsFullScreen = 0; // force full screen
    int optionDefaultWidth = 640;
    int optionDefaultHeight = 480;
    int optionScreenWidth = 0; // force displayed width
    int optionScreenHeight = 0; // force displayed height
    int optionScreenLeft = 0; // set the x position for the left of the window
    int optionScreenTop = 0;  // set the y position for the top of the window
    char *optionWantedStreamSpec[AVMEDIA_TYPE_NB] = {nullptr}; // "select desired stream"
    int optionSeekByBytes = -1; // seek by bytes 0=off 1=on -1=auto
    float optionSeekInterval = 10; // set seek interval for left/right keys, in seconds
    int optionStartupVolume = 100; // set startup volume 0=min 100=max
    int optionSyncType = SYNC_TYPE_AUDIO_MASTER; // set audio-video sync. type (type=audio/video/ext)
    int64_t optionStartTime = AV_NOPTS_VALUE; // seek to a given position in seconds
    int64_t optionDuration = AV_NOPTS_VALUE; // play  \"duration\" seconds of audio/video
    int optionBrorderless = 0; // borderless window
    int optionFast = 0; // non spec compliant optimizations
    int optionGeneratePts = 0; // generate pts
    int optionLowres = 0;
    int optionAutoRotate = 0; // automatically rotate video
    int optionDecoderReorderPts = -1; // let decoder reorder pts 0=off 1=on -1=auto
    int optionAutoExit; // exit at the end
    int optionLoop = 1; // set number of times the playback shall be looped
    int optionDropFrameWhenSlow = -1; // drop frames when cpu is too slow
    int optionShowMode = SHOW_MODE_VIDEO; // select show mode (0 = video, 1 = waves, 2 = RDFT)
    char *optionInputFormatName = nullptr;
    char *optionAudioCodecName = nullptr; // force decoder
    char *optionSubtitleCodecName = nullptr; // force decoder
    char *optionVideoCodecName = nullptr; // force decoder
    double optionRdftSpeed = 0.02;  // rdft speed
    int optionInfiniteBuffer = -1; // don't limit the input buffer size (useful with realtime streams)
    int optionShowStatus = 1; // show status
    int optionFindStreamInfo = 1; // read and decode the streams to fill missing information with heuristics

public:
    FFPlay();

    ~FFPlay();

    Surface *getSurface() const;

    void setAudio(Audio *audio);

    void setSurface(Surface *surface);

    void setStream(Stream *stream);

    MessageQueue *getMsgQueue() const;

    int stop();

    int shutdown();

    int waitStop();

    int prepareStream(const char *fileName);

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

    double frameDuration(Frame *currentFrame, Frame *nextFrame);

    double computeTargetDelay(double delay);

    void updateVideoClockPts(double pts, int64_t pos, int serial);

    void syncClockToSlave(Clock *c, Clock *slave);

    int videoOpen();

    void videoImageDisplay();

    int uploadTexture(AVFrame *pFrame);
};

#endif //SPLAYER_PLAY_H
