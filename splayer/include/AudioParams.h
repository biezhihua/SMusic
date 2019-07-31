//
// Created by biezhihua on 2019-07-16.
//

#ifndef ANDROID_SPLAYER_AUDIOPARAMS_H
#define ANDROID_SPLAYER_AUDIOPARAMS_H


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

class AudioParams {
public:
    int freq;
    int channels;
    int64_t channel_layout;
    enum AVSampleFormat fmt;
    int frame_size;
    int bytes_per_sec;
};

#endif //ANDROID_SPLAYER_AUDIOPARAMS_H
