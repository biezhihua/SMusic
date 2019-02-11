//
// Created by biezhihua on 2019/2/11.
//

#ifndef SMUSIC_SOPENSLES_H
#define SMUSIC_SOPENSLES_H

#include <string>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

class SOpenSLES {

private:

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
    // a mutext to guard against re-entrance to record & playback
    // as well as make recording and playing back to be mutually exclusive
    // this is to avoid crash at situations like:
    //    recording is in session [not finished]
    //    user presses record button and another recording coming in
    // The action: when recording/playing back is not finished, ignore the new request
    static pthread_mutex_t audioEngineLock = PTHREAD_MUTEX_INITIALIZER;

private:
    void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context);
public:
    SOpenSLES();

    ~SOpenSLES();

    void createEngine();

    void createBufferQueueAudioPlayer();
};


#endif //SMUSIC_SOPENSLES_H
