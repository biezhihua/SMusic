#ifndef SPLAYER_AOUT_H
#define SPLAYER_AOUT_H

class Stream;

#include "Log.h"
#include "Mutex.h"
#include "Stream.h"
#include "AudioParams.h"
#include "Define.h"
#include "Options.h"

extern "C" {
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
}

#define AUDIO_TAG "Audio"

class Audio {

protected:
    Stream *stream = nullptr;
    Mutex *mutex = nullptr;
    Options *options = nullptr;

public:
    Audio();

    virtual ~Audio();

    virtual int create() = 0;

    virtual int destroy() = 0;

    void setStream(Stream *stream);

    virtual int openAudio(int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, AudioParams *audio_hw_params);

    virtual void pauseAudio();

    int audioDecodeFrame();

    int synchronizeAudio(int nbSamples);

    void setOptions(Options *options);
};


#endif //SPLAYER_AOUT_H
