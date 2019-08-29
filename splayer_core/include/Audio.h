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
#include "Frame.h"
#include "VideoState.h"

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

    /// 音频回调时间
    int64_t audioCallbackTime;

public:
    Audio();

    virtual ~Audio();

    virtual int create() = 0;

    virtual int destroy() = 0;

    void setStream(Stream *stream);

    virtual int openAudio(int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate,
                          AudioParams *audio_hw_params);

    virtual void pauseAudio();

    virtual void audioCallback(uint8_t *stream, int streamLength);

    int audioDecodeFrame();

    int synchronizeAudio(int nbSamples);

    void setOptions(Options *options);

    void setMsgQueue(MessageQueue *msgQueue);

    void setMediaPlayer(MediaPlayer *mediaPlayer);

    uint64_t getWantedChannelLayout(const Frame *frame) const;

    int initConvertSwrContext(VideoState *is, int64_t desireChannelLayout, const Frame *frame) const;

    int convertAudio(VideoState *is, int wantedNbSamples, Frame *frame);

    void update_sample_display(short *samples, int samples_size);

    virtual void maxAudio(uint8_t *stream, uint8_t *buf, int index, int length, int volume);

    void updateVolume(int sign, double step) const;

};


#endif //SPLAYER_CORE_AUDIO_H
