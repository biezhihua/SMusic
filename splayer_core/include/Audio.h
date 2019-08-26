#ifndef SPLAYER_CORE_AUDIO_H
#define SPLAYER_CORE_AUDIO_H

class Stream;

class MediaPlayer;


#include "Log.h"
#include "Mutex.h"
#include "Stream.h"
#include "AudioParams.h"
#include "Define.h"
#include "Options.h"
#include "MediaPlayer.h"

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
    MessageQueue *msgQueue = nullptr;
    MediaPlayer *mediaPlayer = nullptr;

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

    void setMsgQueue(MessageQueue *msgQueue);

    void setMediaPlayer(MediaPlayer *mediaPlayer);
};


#endif //SPLAYER_CORE_AUDIO_H
