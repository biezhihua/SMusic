//
// Created by biezhihua on 2019/2/11.
//

#include "SOpenSLES.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

SOpenSLES::SOpenSLES(SFFmpeg *pFFmpeg, SStatus *pStatus, SJavaMethods *pJavaMethods) {
    this->pFFmpeg = pFFmpeg;
    this->pStatus = pStatus;
    this->pJavaMethods = pJavaMethods;
}

SOpenSLES::~SOpenSLES() {
    this->pJavaMethods = NULL;
    this->pFFmpeg = NULL;
    this->pStatus = NULL;
}

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {

    SOpenSLES *pOpenSLES = (SOpenSLES *) context;

    // this callback handler is called every time a buffer finishes playing
    if (pOpenSLES != NULL &&
        pOpenSLES->pFFmpeg != NULL &&
        pOpenSLES->bqPlayerBufferQueue != NULL
            ) {

        int result = pOpenSLES->blockResampleAudio();
        if (result >= 0) {
            pOpenSLES->nextAudioSize = static_cast<unsigned int>(result);
            pOpenSLES->pNextAudioBuffer = reinterpret_cast<uint8_t *>(pOpenSLES->pSoundNextBuffer);
            if (pOpenSLES->nextAudioSize != 0) {
                (*pOpenSLES->bqPlayerBufferQueue)->Enqueue(pOpenSLES->bqPlayerBufferQueue,
                                                           pOpenSLES->pNextAudioBuffer,
                                                           pOpenSLES->nextAudioSize);
            }
        }
    }
}

int SOpenSLES::init(int sampleRate) {

    this->sampleRate = sampleRate;

    int result;

    result = initSoundTouch();
    if (result != S_SUCCESS) {
        return result;
    }

    result = initOpenSLES();
    if (result != S_SUCCESS) {
        return result;
    }

    return result;
}

int SOpenSLES::initOpenSLES() {

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
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, (SLuint32) getOpenSLESSampleRate(sampleRate),
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
    const SLInterfaceID ids2[5] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_EFFECTSEND, SL_IID_MUTESOLO,
                                   SL_IID_PLAYBACKRATE};
    const SLboolean req2[5] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
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

    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_MUTESOLO, &bqPlayerMuteSolo);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("SOpenSLES: init: bqPlayerObject SL_IID_MUTESOLO failed");
    }

    // get the volume interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("SOpenSLES: init: bqPlayerObject SL_IID_VOLUME failed");
        return S_ERROR;
    }

    volume(50);
    mute(2);

    bqPlayerCallback(bqPlayerBufferQueue, this);

    return S_SUCCESS;
}


int SOpenSLES::initSoundTouch() {
    pSoundNextBuffer = static_cast<SAMPLETYPE *>(malloc(static_cast<size_t>(sampleRate * 2 * 2)));
    pSoundTouch = new SoundTouch();
    pSoundTouch->setSampleRate(static_cast<uint>(sampleRate));
    pSoundTouch->setChannels(2);
    pSoundTouch->setPitch(soundPitch);
    pSoundTouch->setTempo(soundSpeed);
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


void SOpenSLES::volume(int percent) {
    if (bqPlayerVolume != NULL) {
        currentVolume = (100 - percent) * -50;
        LOGD("SOpenSLES:volume: %d %d", percent, currentVolume);
        (*bqPlayerVolume)->SetVolumeLevel(bqPlayerVolume, (SLmillibel) currentVolume);
    }
}

int SOpenSLES::blockResampleAudio() {
    int result = 0;
    while (pStatus->isLeastActiveState(STATE_PRE_PLAY)) {

        // Seek
        if (pStatus != NULL && pStatus->isSeek()) {
            pFFmpeg->sleep();
            continue;
        }

        // Load State
        if (pFFmpeg->getAudioQueue() != NULL && pFFmpeg->getAudioQueue()->getSize() == 0) {
            if (!isLoading) {
                isLoading = true;
                if (pJavaMethods != NULL) {
                    pJavaMethods->onCallJavaLoadState(true);
                }
            }
            pFFmpeg->sleep();
            continue;
        } else {
            if (isLoading) {
                isLoading = false;
                if (pJavaMethods != NULL) {
                    pJavaMethods->onCallJavaLoadState(false);
                }
            }
        }

        // Sound Touch Receive
        if (soundSamples != 0) {
            soundSamples = pSoundTouch->receiveSamples(pSoundNextBuffer, (uint) (audioDataSize / 4));
            if (soundSamples != 0) {
                return soundSamples * 2 * 2;
            }
        }

        result = pFFmpeg->resampleAudio();

        if (result == S_ERROR_BREAK) {
            return result;
        } else if (result == S_ERROR_CONTINUE) {
            continue;
        } else if (result >= 0) {
            if (result > 0) {
                // Transform Audio Data To SoundTouch Data
                audioDataSize = result;
                for (int i = 0; i < audioDataSize / 2 + 1; i++) {
                    pSoundNextBuffer[i] = (pFFmpeg->getBuffer()[i * 2] | ((pFFmpeg->getBuffer()[i * 2 + 1]) << 8));
                }
                if (pSoundTouch != NULL) {
                    pSoundTouch->putSamples(pSoundNextBuffer, (uint) pFFmpeg->getChannelSampleNumbers());
                    soundSamples = pSoundTouch->receiveSamples(pSoundNextBuffer, (uint) (audioDataSize / 4));
                    if (soundSamples == 0) {
                        continue;
                    }
                    return soundSamples * 2 * 2;
                }
            } else {
                LOGD("SOpenSLES:blockResampleAudio: resample end");
                if (pSoundTouch != NULL) {
                    pSoundTouch->flush();
                }
                return 0;
            }
            return result;
        }
    }
    return result;
}

int SOpenSLES::getOpenSLESSampleRate(int sampleRate) {
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

    stop();

    if (pSoundTouch != NULL) {
        delete pSoundTouch;
        pSoundTouch = NULL;
        delete pSoundNextBuffer;
        pSoundNextBuffer = NULL;
    }

    if (pNextAudioBuffer != NULL) {
        delete pNextAudioBuffer;
        pNextAudioBuffer = NULL;
    }

    if (pSoundNextBuffer != NULL) {
        delete pSoundNextBuffer;
        pSoundNextBuffer = NULL;
    }

    if (bqPlayerObject != NULL) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = NULL;
        bqPlayerPlay = NULL;
        bqPlayerBufferQueue = NULL;
        bqPlayerVolume = NULL;
        bqPlayerMuteSolo = NULL;
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

jint SOpenSLES::getCurrentVolumePercent() {
    return currentVolume / -50;
}

void SOpenSLES::mute(int mute) {
    if (bqPlayerMuteSolo != NULL) {
        switch (mute) {
            case 0:
                // Left
                (*bqPlayerMuteSolo)->SetChannelMute(bqPlayerMuteSolo, 1, static_cast<SLboolean>(true));
                (*bqPlayerMuteSolo)->SetChannelMute(bqPlayerMuteSolo, 0, static_cast<SLboolean>(false));
                break;
            case 1:
                // Right
                (*bqPlayerMuteSolo)->SetChannelMute(bqPlayerMuteSolo, 1, static_cast<SLboolean>(false));
                (*bqPlayerMuteSolo)->SetChannelMute(bqPlayerMuteSolo, 0, static_cast<SLboolean>(true));
                break;
            case 2:
                // Center
                (*bqPlayerMuteSolo)->SetChannelMute(bqPlayerMuteSolo, 1, static_cast<SLboolean>(false));
                (*bqPlayerMuteSolo)->SetChannelMute(bqPlayerMuteSolo, 0, static_cast<SLboolean>(false));
                break;
            default:
                break;
        }
    }
}

void SOpenSLES::setSoundTouchPitch(double soundPitch) {
    LOGD("SOpenSLES:setSoundTouchPitch: %f", soundPitch);
    this->soundPitch = soundPitch;
    if (pSoundTouch != NULL) {
        pSoundTouch->setPitch(soundPitch);
    }
}

void SOpenSLES::setSoundTouchTempo(double soundSpeed) {
    LOGD("SOpenSLES:setSoundTouchTempo: %f", soundSpeed);
    this->soundSpeed = soundSpeed;
    if (pSoundTouch != NULL) {
        pSoundTouch->setTempo(soundSpeed);
    }
}

double SOpenSLES::getSoundSpeed() const {
    return soundSpeed;
}

double SOpenSLES::getSoundPitch() const {
    return soundPitch;
}


#pragma clang diagnostic pop