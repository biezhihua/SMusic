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
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
};

//enum {
//    AV_SYNC_AUDIO_MASTER, /* default choice */
//    AV_SYNC_VIDEO_MASTER,
//    AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
//};

//static AVDictionary *format_opts, *codec_opts, *resample_opts;
///* options specified by the user */
//static AVInputFormat *file_iformat;
//static const char *input_filename;
//static const char *window_title;
//static int default_width = 640;
//static int default_height = 480;
//static int screen_width = 0;
//static int screen_height = 0;
//static int screen_left = 0;
//static int screen_top = 0;
//static int audio_disable;
//static int video_disable;
//static int subtitle_disable;
//static const char *wanted_stream_spec[AVMEDIA_TYPE_NB] = {0};
//static int seek_by_bytes = -1;
//static float seek_interval = 10;
//static int display_disable;
//static int borderless;
//static int startup_volume = 100;
//static int show_status = 1;
//static int av_sync_type = AV_SYNC_AUDIO_MASTER;
//static int64_t start_time = AV_NOPTS_VALUE;
//static int64_t duration = AV_NOPTS_VALUE;
//static int fast = 0;
//static int genpts = 0;
//static int lowres = 0;
//static int decoder_reorder_pts = -1;
//static int autoexit;
//static int exit_on_keydown;
//static int exit_on_mousedown;
//static int loop = 1;
//static int framedrop = -1;
//static int infinite_buffer = -1;
////static int show_mode = SHOW_MODE_NONE;
//static const char *audio_codec_name;
//static const char *subtitle_codec_name;
//static const char *video_codec_name;
//static double rdftspeed = 0.02;
//static int64_t cursor_last_shown;
//static int cursor_hidden = 0;
//static int autorotate = 1;
//static int find_stream_info = 1;
//
///* current context */
//static int is_full_screen;
//static int64_t audio_callback_time;
//
//static AVPacket flush_pkt;

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

    char *inputFileName = nullptr;

    VideoState *videoState = nullptr;

    int videoQueueSize;

    int startupVolume = 100;

    int avSyncType = AV_SYNC_AUDIO_MASTER;

    int startOnPrepared;

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

    void readThread();

private:
    void showVersionsAndOptions();

    void showDict(const char *tag, AVDictionary *dict);

    VideoState *streamOpen(const char *fileName, AVInputFormat *inputFormat);

    void streamClose();

    int frameQueueInit(FrameQueue *pFrameQueue, PacketQueue *pPacketQueue, int queueSize, int keepLast);

    int packetQueueInit(PacketQueue *pQueue);

    int initClock(Clock *pClock, int *pQueueSerial);

    void setClock(Clock *pClock, double pts, int serial);

    void setClockAt(Clock *pClock, double pts, int serial, double time);


    int getStartupVolume();
};

#endif //SPLAYER_PLAY_H

#pragma clang diagnostic pop