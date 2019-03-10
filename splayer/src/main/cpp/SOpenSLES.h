//
// Created by biezhihua on 2019/2/11.
//

#ifndef SMUSIC_SOPENSLES_H
#define SMUSIC_SOPENSLES_H

#include <string>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "SLog.h"
#include "SMedia.h"
#include "SFFmpeg.h"
#include <pthread.h>

#include "SoundTouch.h"

using namespace soundtouch;

class SOpenSLES {

private:
    bool isLoading = false;

    SJavaMethods *pJavaMethods = NULL;

    // SoundTouch
    SoundTouch *pSoundTouch = NULL;

    int currentVolume = -2500;
    int sampleRate = 0;

    double soundSpeed = 1.0f;
    double soundPitch = 1.0f;
    int soundSamples = 0;
    int audioDataSize = 0;

public:

    int state = SL_PLAYSTATE_STOPPED;

    SFFmpeg *pFFmpeg = NULL;

    SStatus *pStatus = NULL;

    // engine interfaces
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine;

    // output mix interfaces
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

    // aux effect on the output mix, used by the buffer queue player
    const SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    // buffer queue player interfaces
    SLObjectItf bqPlayerObject = NULL;
    SLPlayItf bqPlayerPlay;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;

    SLVolumeItf bqPlayerVolume = NULL;
    SLMuteSoloItf bqPlayerMuteSolo = NULL;

    uint8_t *pNextAudioBuffer = NULL;
    SAMPLETYPE *pSoundNextBuffer = NULL;
    unsigned nextAudioSize;

public:

    SOpenSLES(SFFmpeg *pFFmpeg, SStatus *pSStatus, SJavaMethods *pJavaMethods);

    ~SOpenSLES();

    int init(int sampleRate);

    int play();

    int pause();

    int stop();

    int release();

    int blockResampleAudio();

    int getOpenSLESSampleRate(int sampleRate);

    void volume(int percent);

    jint getCurrentVolumePercent();

    void mute(int mute);

    int initSoundTouch();

    int initOpenSLES();

    void setSoundTouchPitch(double soundPitch);

    void setSoundTouchTempo(double soundSpeed);

    double getSoundSpeed() const;

    double getSoundPitch() const;


};


#endif //SMUSIC_SOPENSLES_H
