#ifndef SPLAYER_AOUT_H
#define SPLAYER_AOUT_H

class Stream;

#include "Log.h"
#include "Mutex.h"
#include "Stream.h"
#include "AudioParams.h"

class Audio {

protected:
    Stream *stream = nullptr;
    Mutex *mutex = nullptr;

public:
    Audio();

    virtual ~Audio();

    virtual int create() = 0;

    virtual int destroy() = 0;

    void setStream(Stream *stream);

    virtual int openAudio(int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, AudioParams *audio_hw_params);

};


#endif //SPLAYER_AOUT_H
