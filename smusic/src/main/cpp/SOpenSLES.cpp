//
// Created by biezhihua on 2019/2/11.
//

#include "SOpenSLES.h"

SOpenSLES::SOpenSLES(SFFmpeg *pFFmpeg, SStatus *pStatus) {
    this->pFFmpeg = pFFmpeg;
    this->pStatus = pStatus;
}

SOpenSLES::~SOpenSLES() {
    this->pFFmpeg = NULL;
    this->pStatus = NULL;
}

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {

    SOpenSLES *pOpenSLES = (SOpenSLES *) context;

    // this callback handler is called every time a buffer finishes playing
    if (pOpenSLES != NULL && pOpenSLES->pFFmpeg != NULL &&
        pOpenSLES->bqPlayerBufferQueue != NULL) {
        int result = pOpenSLES->resampleAudio();
        if (result >= 0) {
            pOpenSLES->nextSize = static_cast<unsigned int>(result);
            pOpenSLES->nextBuffer = pOpenSLES->pFFmpeg->getBuffer();
            if (pOpenSLES->nextSize != 0) {
                // LOGD("OpenSLES: bqPlayerCallback: Enqueue");
                (*pOpenSLES->bqPlayerBufferQueue)->Enqueue(pOpenSLES->bqPlayerBufferQueue,
                                                           pOpenSLES->nextBuffer,
                                                           pOpenSLES->nextSize);
            }
        }
    }
}

int SOpenSLES::init(int sampleRate) {

    SLresult result;

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("SOpenSLES: init: slCreateEngine failed");
        return S_ERROR;
    }

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("SOpenSLES: init: engineObject Realize failed");
        return S_ERROR;
    }

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("SOpenSLES: init: engineObject GetInterface engineEngine failed");
        return S_ERROR;
    }

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("SOpenSLES: init: engineEngine CreateOutputMix outputMixObject failed");
        return S_ERROR;
    }

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("SOpenSLES: init: outputMixObject Realize failed");
        return S_ERROR;
    }

    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*outputMixObject)->GetInterface(outputMixObject,
                                              SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("SOpenSLES: init: outputMixObject GetInterface SL_IID_ENVIRONMENTALREVERB failed");
        return S_ERROR;
    }

    (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(outputMixEnvironmentalReverb, &reverbSettings);

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, (SLuint32) getSampleRate(sampleRate),
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};
    /*
     * Enable Fast Audio when possible:  once we set the same rate to be the native, fast audio path
     * will be triggered
     */
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    /*
     * create audio player:
     *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
     *     for fast audio case
     */
    const SLInterfaceID ids2[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_EFFECTSEND,/*SL_IID_MUTESOLO,*/};
    const SLboolean req2[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,/*SL_BOOLEAN_TRUE,*/ };
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk, 2, ids2, req2);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("SOpenSLES: init: engineEngine CreateAudioPlayer bqPlayerObject failed");
        return S_ERROR;
    }

    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("SOpenSLES: init: bqPlayerObject Realize failed");
        return S_ERROR;
    }

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("SOpenSLES: init: bqPlayerObject GetInterface SL_IID_PLAY failed");
        return S_ERROR;
    }

    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("SOpenSLES: init: bqPlayerObject GetInterface SL_IID_BUFFERQUEUE failed");
        return S_ERROR;
    }

    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("SOpenSLES: init: bqPlayerBufferQueue RegisterCallback failed");
        return S_ERROR;
    }

    bqPlayerCallback(bqPlayerBufferQueue, this);

    return S_SUCCESS;
}

int SOpenSLES::play() {
    LOGD("OpenSLES: play");
    if (state != SL_PLAYSTATE_PLAYING && bqPlayerPlay != NULL) {
        state = SL_PLAYSTATE_PLAYING;
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
        return S_SUCCESS;
    }
    return S_ERROR;
}

int SOpenSLES::pause() {
    LOGD("OpenSLES: pause");
    if (state != SL_PLAYSTATE_PAUSED && bqPlayerPlay != NULL) {
        state = SL_PLAYSTATE_PAUSED;
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PAUSED);
        return S_SUCCESS;
    }
    return S_ERROR;
}

int SOpenSLES::stop() {
    LOGD("OpenSLES: stop");
    if (state != SL_PLAYSTATE_STOPPED && bqPlayerPlay != NULL) {
        state = SL_PLAYSTATE_STOPPED;
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
        return S_SUCCESS;
    }
    return S_ERROR;
}

int SOpenSLES::resampleAudio() {
    int result = 0;
    while (pStatus->isLeastActiveState(STATE_PRE_PLAY)) {
        result = pFFmpeg->resampleAudio();
        // LOGD("SOpenSLES: resampleAudio: result = %d", result);
        if (result == S_ERROR_BREAK) {
            return result;
        } else if (result == S_ERROR_CONTINUE) {
            continue;
        } else if (result >= 0) {
            return result;
        }
    }
    return result;
}

int SOpenSLES::getSampleRate(int sampleRate) {
    int rate = 0;
    switch (sampleRate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }
    return rate;
}

int SOpenSLES::release() {

    LOGD("OpenSLES: release");

    if (bqPlayerObject != NULL) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = NULL;
        bqPlayerPlay = NULL;
        bqPlayerBufferQueue = NULL;
    }

    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }

    if (engineObject != NULL) {
        engineObject = NULL;
        engineEngine = NULL;
    }

    return 0;
}



