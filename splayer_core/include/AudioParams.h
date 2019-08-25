#ifndef SPLAYER_CORE_AUDIO_PARAMS_H
#define SPLAYER_CORE_AUDIO_PARAMS_H

extern "C" {
#include <libavutil/samplefmt.h>
};

class AudioParams {
public:
    int freq;
    int channels;
    int64_t channel_layout;
    AVSampleFormat fmt;
    int frame_size;
    int bytes_per_sec;
};


#endif //SPLAYER_CORE_AUDIO_PARAMS_H
