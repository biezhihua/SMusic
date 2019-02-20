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

class SOpenSLES {

private:
    SJavaMethods *pJavaMethods = NULL;

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

    uint8_t *nextBuffer = NULL;
    unsigned nextSize;

public:

    SOpenSLES(SFFmpeg *pFFmpeg, SStatus *pSStatus, SJavaMethods *pJavaMethods);

    ~SOpenSLES();

    int init(int sampleRate);

    int play();

    int pause();

    int stop();

    int release();

    int resampleAudio();

    int getSampleRate(int sampleRate);
};


#endif //SMUSIC_SOPENSLES_H
