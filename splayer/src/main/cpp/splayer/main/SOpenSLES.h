#ifndef SPLAYER_S_OPENSLES_H
#define SPLAYER_S_OPENSLES_H

#include <string>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "SLog.h"
#include "SMedia.h"
#include "SFFmpeg.h"
#include <pthread.h>

#include "../../soundtouch/SoundTouch.h"

using namespace soundtouch;

class SOpenSLES {

private:
    bool isLoading = false;

    SJavaMethods *pJavaMethods = nullptr;

    // SoundTouch
    SoundTouch *pSoundTouch = nullptr;

    int currentVolume = -2500;
    int sampleRate = 0;

    double soundSpeed = 1.0f;
    double soundPitch = 1.0f;
    int soundSamples = 0;
    int audioDataSize = 0;

public:

    int state = SL_PLAYSTATE_STOPPED;

    SFFmpeg *pFFmpeg = nullptr;

    SStatus *pStatus = nullptr;

    // engine interfaces
    SLObjectItf engineObject = nullptr;
    SLEngineItf engineEngine;

    // output mix interfaces
    SLObjectItf outputMixObject = nullptr;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = nullptr;

    // aux effect on the output mix, used by the buffer pQueue player
    const SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    // buffer pQueue player interfaces
    SLObjectItf bqPlayerObject = nullptr;
    SLPlayItf bqPlayerPlay;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;

    SLVolumeItf bqPlayerVolume = nullptr;
    SLMuteSoloItf bqPlayerMuteSolo = nullptr;

    uint8_t *pNextAudioBuffer = nullptr;
    SAMPLETYPE *pSoundNextBuffer = nullptr;
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


#endif //SPLAYER_S_OPENSLES_H
