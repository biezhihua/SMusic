#ifndef SPLAYER_CORE_AUDIO_PARAMS_H
#define SPLAYER_CORE_AUDIO_PARAMS_H

extern "C" {
#include <libavutil/samplefmt.h>
};

class AudioParams {
public:

    /// 采样率
    int sampleRate;

    /// 音频通道的数量
    int channels;

    /// 音频数据的通道布局
    int64_t channelLayout;

    /// 音频采样格式
    AVSampleFormat sampleFormat;

    /// 音频缓冲区大小
    int frameSize;

    /// 每秒音频Byte
    int bytesPerSec;
};

#endif
