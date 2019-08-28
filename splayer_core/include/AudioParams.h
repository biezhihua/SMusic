#ifndef SPLAYER_CORE_AUDIO_PARAMS_H
#define SPLAYER_CORE_AUDIO_PARAMS_H

extern "C" {
#include <libavutil/samplefmt.h>
};

class AudioParams {
public:

    /// 采样率
    int sampleRate;

    /// 声道数
    int channels;

    /// 声道设计，单声道，双声道还是立体声
    int64_t channelLayout;

    /// 采样格式
    AVSampleFormat sampleFormat;

    /// 采样大小
    int frameSize;

    /// 每秒多少字节
    int bytesPerSec;
};

#endif
