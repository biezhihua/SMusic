#ifndef SPLAYER_PLAY_H
#define SPLAYER_PLAY_H

#include "Log.h"
#include "Mutex.h"
#include "MessageQueue.h"
#include "AOut.h"
#include "VOut.h"
#include "Pipeline.h"
#include "State.h"
#include "Common.h"
#include "Audio.h"
#include "Clock.h"
#include "PacketQueue.h"
#include "FrameQueue.h"
#include "VideoState.h"

enum {
    AV_SYNC_AUDIO_MASTER, /* default choice */
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};

static AVDictionary *format_opts, *codec_opts, *resample_opts;
/* options specified by the user */
static AVInputFormat *file_iformat;
static const char *input_filename;
static const char *window_title;
static int default_width = 640;
static int default_height = 480;
static int screen_width = 0;
static int screen_height = 0;
static int screen_left = 0;
static int screen_top = 0;
static int audio_disable;
static int video_disable;
static int subtitle_disable;
static const char *wanted_stream_spec[AVMEDIA_TYPE_NB] = {0};
static int seek_by_bytes = -1;
static float seek_interval = 10;
static int display_disable;
static int borderless;
static int startup_volume = 100;
static int show_status = 1;
static int av_sync_type = AV_SYNC_AUDIO_MASTER;
static int64_t start_time = AV_NOPTS_VALUE;
static int64_t duration = AV_NOPTS_VALUE;
static int fast = 0;
static int genpts = 0;
static int lowres = 0;
static int decoder_reorder_pts = -1;
static int autoexit;
static int exit_on_keydown;
static int exit_on_mousedown;
static int loop = 1;
static int framedrop = -1;
static int infinite_buffer = -1;
static int show_mode = SHOW_MODE_NONE;
static const char *audio_codec_name;
static const char *subtitle_codec_name;
static const char *video_codec_name;
static double rdftspeed = 0.02;
static int64_t cursor_last_shown;
static int cursor_hidden = 0;
static int autorotate = 1;
static int find_stream_info = 1;

/* current context */
static int is_full_screen;
static int64_t audio_callback_time;

static AVPacket flush_pkt;

class FFPlay {

private:

    Mutex *avMutex = nullptr;
    Mutex *vfMutex = nullptr;
    MessageQueue *msgQueue = nullptr;
    AOut *aOut = nullptr;
    VOut *vOut = nullptr;
    Pipeline *pipeline = nullptr;
    VideoState *videoState = nullptr;

    char *inputFileName = nullptr;

private:

    VideoState *streamOpen(const char *name, AVInputFormat *inputFormat);

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

    int frame_queue_init(FrameQueue *f, PacketQueue *pktq, int max_size, int keep_last);

    int packet_queue_init(PacketQueue *q);

    void init_clock(Clock *c, int *queue_serial);

    void set_clock(Clock *c, double pts, int serial);

    void set_clock_at(Clock *c, double pts, int serial, double time);
};


#endif //SPLAYER_PLAY_H
