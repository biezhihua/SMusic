//
// Created by biezhihua on 2019/2/4.
//

#include "SPlayer.h"

void *startDecodeAudioFrameCallback(void *data) {
    LOGD("SPlayer: startDecodeAudioFrameCallback: start");
    SPlayer *sPlayer = (SPlayer *) data;
    if (sPlayer != NULL) {

        SFFmpeg *pFFmpeg = sPlayer->getSFFmpeg();
        SStatus *pPlayerStatus = sPlayer->getPlayerStatus();
        SQueue *pAudioQueue = pFFmpeg->getAudioQueue();

        if (pPlayerStatus != NULL && pAudioQueue != NULL) {
            while (pPlayerStatus->isLeastActiveState(STATE_PRE_PLAY)) {
                int result = pFFmpeg->decodeAudioFrame();
                if (result == S_ERROR_BREAK) {
                    while (!pPlayerStatus->isLeastActiveState(STATE_PLAY)) {
                        if (pAudioQueue->getSize() > 0) {
                            continue;
                        } else {
                            break;
                        }
                    }
                }
            }
        }
        LOGD("SPlayer: startDecodeAudioFrameCallback: end");
        pthread_exit(&sPlayer->startDecodeAudioThread);
    }
    return NULL;
}

void *startDecodeMediaInfoCallback(void *data) {
    LOGD("SPlayer: startDecodeMediaInfoCallback: start");
    SPlayer *sPlayer = (SPlayer *) data;
    if (sPlayer != NULL) {
        if (sPlayer->getSFFmpeg() != NULL) {
            SFFmpeg *pSFFmpeg = sPlayer->getSFFmpeg();
            SStatus *pSStatus = sPlayer->getPlayerStatus();
            int result = pSFFmpeg->decodeMediaInfo();
            if (result == S_SUCCESS) {
                pSStatus->moveStatusToStart();
                sPlayer->getSJavaMethods()->onCallJavaStart();
            } else {
                // TODO Release
                pSStatus->moveStatusToStop();
                // TODO ERROR
                LOGE("SPlayer:  startDecodeMediaInfoCallback error");
            }
        }
        LOGD("SPlayer: startDecodeMediaInfoCallback: end");
        pthread_exit(&sPlayer->startDecodeMediaInfoThread);
    }
    return NULL;
}

void *playAudioCallback(void *data) {
    LOGD("SPlayer: playAudioCallback: start");
    SPlayer *sPlayer = (SPlayer *) data;
    if (sPlayer != NULL && sPlayer->getSFFmpeg() != NULL) {
        SFFmpeg *pFFmpeg = sPlayer->getSFFmpeg();
        SStatus *pPlayerStatus = sPlayer->getPlayerStatus();
        SOpenSLES *pOpenSLES = sPlayer->getSOpenSLES();
        if (pPlayerStatus != NULL &&
            pFFmpeg != NULL &&
            pOpenSLES != NULL &&
            pPlayerStatus->isLeastActiveState(STATE_PRE_PLAY)) {

            LOGD("SPlayer: playAudioCallback called: Init OpenSLES");
            pOpenSLES->createEngine();
            SMedia *audio = pFFmpeg->getAudio();
            int sampleRate = 0;
            if (audio != NULL) {
                sampleRate = audio->getSampleRate();
            }
            pOpenSLES->createBufferQueueAudioPlayer(sampleRate);
            pOpenSLES->play();

            pPlayerStatus->moveStatusToPlay();
        }
        LOGD("SPlayer: playAudioCallback: end");
        pthread_exit(&sPlayer->playAudioThread);
    }
    return NULL;
}

SPlayer::SPlayer(JavaVM *pVm, JNIEnv *env, jobject instance, SJavaMethods *pMethods) {
    pJavaVM = pVm;
    mainJniEnv = env;
    javaInstance = mainJniEnv->NewGlobalRef(instance);
    pJavaMethods = pMethods;

    create();
}

SPlayer::~SPlayer() {

    destroy();

    pJavaMethods = NULL;
    mainJniEnv->DeleteGlobalRef(javaInstance);
    javaInstance = NULL;
    mainJniEnv = NULL;
    pJavaVM = NULL;
}


void SPlayer::create() {
    pStatus = new SStatus();
    pFFmpeg = new SFFmpeg(pStatus);
    pOpenSLES = new SOpenSLES(pFFmpeg, pStatus);

    if (pStatus != NULL) {
        pStatus->moveStatusToCreate();
    }
    if (pJavaMethods != NULL) {
        pJavaMethods->onCallJavaCreate();
    }
}


void SPlayer::setSource(string *url) {
    if (pStatus->isCreate() || pStatus->isStop()) {
        pSource = url;
        if (pFFmpeg != NULL) {
            pFFmpeg->setSource(url);
        }
        pStatus->moveStatusToSource();
    }
}

void SPlayer::start() {
    if (pStatus->isSource() || pStatus->isStop()) {
        pStatus->moveStatusToPreStart();
        pthread_create(&startDecodeMediaInfoThread, NULL, startDecodeMediaInfoCallback, this);
    }
}

void SPlayer::play() {
    if (pStatus->isStart()) {
        pStatus->moveStatusToPrePlay();
        pthread_create(&playAudioThread, NULL, playAudioCallback, this);
        pthread_create(&startDecodeAudioThread, NULL, startDecodeAudioFrameCallback, this);
    } else if (pStatus->isPause()) {
        pOpenSLES->play();
        pStatus->moveStatusToPlay();
    }
}

void SPlayer::pause() {
    if (pStatus->isPlay()) {
        pOpenSLES->pause();
        pStatus->moveStatusToPause();
    }
}

void SPlayer::stop() {
    if (pStatus->isLeastActiveState(STATE_SOURCE)) {
        pOpenSLES->stop();
        pFFmpeg->reset();
        pStatus->moveStatusToStop();
    }
}

void SPlayer::destroy() {
    delete pOpenSLES;
    pOpenSLES = NULL;

    delete pFFmpeg;
    pFFmpeg = NULL;

    if (pJavaMethods != NULL) {
        pJavaMethods->onCallJavaDestroy();
    }

    if (pStatus != NULL) {
        pStatus->moveStatusToDestroy();
    }

    delete pStatus;
    pStatus = NULL;

    pSource = NULL;
}

SFFmpeg *SPlayer::getSFFmpeg() {
    return pFFmpeg;
}

SJavaMethods *SPlayer::getSJavaMethods() {
    return pJavaMethods;
}


SStatus *SPlayer::getPlayerStatus() {
    return pStatus;
}

SOpenSLES *SPlayer::getSOpenSLES() {
    return pOpenSLES;
}


