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

class SOpenSLES {

public:

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

    short *nextBuffer = NULL;
    unsigned nextSize;
    int nextCount;

public:

    SOpenSLES(SFFmpeg *pFFmpeg, SStatus *pSStatus);

    ~SOpenSLES();

    void createEngine();

    void createBufferQueueAudioPlayer();

    void play();

    void pause();

    void stop();

    int resampleAudio();
};


#endif //SMUSIC_SOPENSLES_H
