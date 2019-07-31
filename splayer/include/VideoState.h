#ifndef SPLAYER2_VIDEOSTATE_H
#define SPLAYER2_VIDEOSTATE_H

extern "C" {
#include <libavutil/time.h>
#include <libavutil/rational.h>
#include <libavutil/mem.h>
#include <libavutil/log.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
};

#include "Thread.h"
#include "Mutex.h"
#include "FFPlay.h"
#include "MyAVPacketList.h"
#include "Clock.h"
#include "Decoder.h"
#include "FrameQueue.h"
#include "AudioParams.h"

class VideoState {

public:

    int show_mode;

    Thread *read_tid;
    AVInputFormat *iformat;
    int abort_request;
    int force_refresh;
    int paused;
    int last_paused;
    int queue_attachments_req;
    int seek_req;
    int seek_flags;
    int64_t seek_pos;
    int64_t seek_rel;
    int read_pause_return;
    AVFormatContext *ic;
    int realtime;

    Clock audclk;
    Clock vidclk;
    Clock extclk;

    FrameQueue pictq;
    FrameQueue subpq;
    FrameQueue sampq;

    Decoder auddec;
    Decoder viddec;
    Decoder subdec;

    int audio_stream;

    int av_sync_type;

    double audio_clock;
    int audio_clock_serial;
    double audio_diff_cum; /* used for AV difference average computation */
    double audio_diff_avg_coef;
    double audio_diff_threshold;
    int audio_diff_avg_count;
    AVStream *audio_st;
    PacketQueue audioq;
    int audio_hw_buf_size;
    uint8_t *audio_buf;
    uint8_t *audio_buf1;
    unsigned int audio_buf_size; /* in bytes */
    unsigned int audio_buf1_size;
    int audio_buf_index; /* in bytes */
    int audio_write_buf_size;
    int audio_volume;
    int muted;
    AudioParams audio_src;
    AudioParams audio_tgt;
    SwrContext *swr_ctx;
    int frame_drops_early;
    int frame_drops_late;

    int16_t sample_array[SAMPLE_ARRAY_SIZE];
    int sample_array_index;
    int last_i_start;
    // RDFTContext *rdft;
    int rdft_bits;
    // FFTSample *rdft_data;
    int xpos;
    double last_vis_time;
    void *vis_texture;
    void *sub_texture;
    void *vid_texture;

    int subtitle_stream;
    AVStream *subtitle_st;
    PacketQueue subtitleq;

    double frame_timer;
    double frame_last_returned_time;
    double frame_last_filter_delay;
    int video_stream;
    AVStream *video_st;
    PacketQueue videoq;
    double max_frame_duration;      // maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
    SwsContext *img_convert_ctx;
    SwsContext *sub_convert_ctx;
    int eof;

    char *filename;
    int width;
    int height;
    int xleft;
    int ytop;
    int step;
    int last_video_stream;
    int last_audio_stream;
    int last_subtitle_stream;

    Mutex *continue_read_thread;
};


#endif //SPLAYER_VIDEOSTATE_H
