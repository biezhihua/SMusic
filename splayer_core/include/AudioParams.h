#ifndef SPLAYER_CORE_AUDIO_PARAMS_H
#define SPLAYER_CORE_AUDIO_PARAMS_H

extern "C" {
#include <libavutil/samplefmt.h>
};

class AudioParams {
public:
    int sampleRate;
    int channels;
    int64_t channelLayout;
    AVSampleFormat sampleFormat;
    int frameSize;
    int bytesPerSec;
};


#endif //SPLAYER_CORE_AUDIO_PARAMS_H
