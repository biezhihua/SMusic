//
// Created by biezhihua on 2019/2/4.
//

#include "SPlayer.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"

void *startDecodeAudioFrameCallback(void *data) {
    LOGD("SPlayer: startDecodeAudioFrameCallback: start");
    SPlayer *sPlayer = (SPlayer *) data;

    if (sPlayer != NULL && sPlayer->getSFFmpeg() != NULL) {

        SFFmpeg *pFFmpeg = sPlayer->getSFFmpeg();
        SStatus *pStatus = sPlayer->getPlayerStatus();
        SOpenSLES *pOpenSLES = sPlayer->getSOpenSLES();
        SJavaMethods *pJavaMethods = sPlayer->getSJavaMethods();
        SQueue *pAudioQueue = pFFmpeg->getAudioQueue();

        if (pStatus != NULL && pAudioQueue != NULL) {
            while (pStatus->isLeastActiveState(STATE_PRE_PLAY)) {
                int result = pFFmpeg->decodeAudioFrame();
                if (result == S_ERROR_BREAK) {
                    while (!pStatus->isLeastActiveState(STATE_PLAY)) {
                        if (pAudioQueue->getSize() > 0) {
                            continue;
                        } else {
                            break;
                        }
                    }
                }
            }
        }

        sPlayer->startDecodeAudioThreadComplete = true;

        if (pOpenSLES != NULL &&
            pFFmpeg != NULL &&
            pStatus != NULL &&
            pStatus->isPreStop() &&
            sPlayer->startDecodeMediaInfoThreadComplete &&
            sPlayer->startDecodeAudioThreadComplete &&
            sPlayer->playAudioThreadComplete) {
            pStatus->moveStatusToStop();
            if (pJavaMethods != NULL) {
                pJavaMethods->onCallJavaStop();
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

    if (sPlayer != NULL && sPlayer->getSFFmpeg() != NULL) {

        SFFmpeg *pFFmpeg = sPlayer->getSFFmpeg();
        SStatus *pStatus = sPlayer->getPlayerStatus();
        SOpenSLES *pOpenSLES = sPlayer->getSOpenSLES();
        SJavaMethods *pJavaMethods = sPlayer->getSJavaMethods();

        int result = pFFmpeg->decodeMediaInfo();
        if (result == S_SUCCESS) {
            pStatus->moveStatusToStart();
            sPlayer->getSJavaMethods()->onCallJavaStart();
        } else {
            // TODO Release
            pStatus->moveStatusToStop();
            // TODO ERROR
            LOGE("SPlayer:  startDecodeMediaInfoCallback error");
        }

        sPlayer->startDecodeMediaInfoThreadComplete = true;

        if (pOpenSLES != NULL &&
            pFFmpeg != NULL &&
            pStatus != NULL &&
            pStatus->isPreStop() &&
            sPlayer->startDecodeMediaInfoThreadComplete &&
            sPlayer->startDecodeAudioThreadComplete &&
            sPlayer->playAudioThreadComplete) {
            pStatus->moveStatusToStop();
            if (pJavaMethods != NULL) {
                pJavaMethods->onCallJavaStop();
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
        SStatus *pStatus = sPlayer->getPlayerStatus();
        SOpenSLES *pOpenSLES = sPlayer->getSOpenSLES();
        SJavaMethods *pJavaMethods = sPlayer->getSJavaMethods();

        if (pStatus != NULL &&
            pFFmpeg != NULL &&
            pOpenSLES != NULL &&
            pStatus->isLeastActiveState(STATE_PRE_PLAY)) {

            LOGD("SPlayer: playAudioCallback called: Init OpenSLES");
            int result = pOpenSLES->createEngine();
            if (result == S_SUCCESS) {
                SMedia *audio = pFFmpeg->getAudio();
                int sampleRate = 0;
                if (audio != NULL) {
                    sampleRate = audio->getSampleRate();
                }
                result = pOpenSLES->createBufferQueueAudioPlayer(sampleRate);
                if (result == S_SUCCESS) {
                    pOpenSLES->play();

                    pStatus->moveStatusToPlay();
                    if (pJavaMethods != NULL) {
                        pJavaMethods->onCallJavaPlay();
                    }
                } else {
                    if (pStatus != NULL &&
                        pStatus->isPrePlay()) {
                        pStatus->moveStatusToStop();
                    }
                }
            } else {
                if (pStatus != NULL &&
                    pStatus->isPrePlay()) {
                    pStatus->moveStatusToStop();
                }
            }
        }

        sPlayer->playAudioThreadComplete = true;

        if (pOpenSLES != NULL &&
            pFFmpeg != NULL &&
            pStatus != NULL &&
            pStatus->isPreStop() &&
            sPlayer->startDecodeMediaInfoThreadComplete &&
            sPlayer->startDecodeAudioThreadComplete &&
            sPlayer->playAudioThreadComplete) {
            pStatus->moveStatusToStop();
            if (pJavaMethods != NULL) {
                pJavaMethods->onCallJavaStop();
            }
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
    LOGD("SPlayer: start: ");

    if (pStatus->isSource() || pStatus->isStop()) {
        pStatus->moveStatusToPreStart();

        startDecodeMediaInfoThreadComplete = false;
        pthread_create(&startDecodeMediaInfoThread, NULL, startDecodeMediaInfoCallback, this);
    }
}

void SPlayer::play() {
    LOGD("SPlayer: play: ");

    if (pStatus->isStart()) {
        pStatus->moveStatusToPrePlay();

        playAudioThreadComplete = false;
        pthread_create(&playAudioThread, NULL, playAudioCallback, this);

        startDecodeAudioThreadComplete = false;
        pthread_create(&startDecodeAudioThread, NULL, startDecodeAudioFrameCallback, this);

    } else if (pStatus->isPause()) {
        pOpenSLES->play();
        pStatus->moveStatusToPlay();
        if (pJavaMethods != NULL) {
            pJavaMethods->onCallJavaPlay();
        }
    }
}

void SPlayer::pause() {
    LOGD("SPlayer: pause: ");

    if (pStatus->isPlay()) {
        pOpenSLES->pause();
        pStatus->moveStatusToPause();
        if (pJavaMethods != NULL) {
            pJavaMethods->onCallJavaPause();
        }
    }
}

void SPlayer::stop() {
    LOGD("SPlayer: stop: ");
    if (pStatus->isLeastActiveState(STATE_SOURCE)) {
        pStatus->moveStatusToPreStop();
        if (pOpenSLES != NULL && pFFmpeg != NULL && pStatus->isPreStop()) {
            bool openSLESResult = pOpenSLES->stop() == S_SUCCESS;
            bool ffmpegResult = pFFmpeg->stop() == S_SUCCESS;
            if (openSLESResult &&
                ffmpegResult &&
                startDecodeMediaInfoThreadComplete &&
                startDecodeAudioThreadComplete &&
                playAudioThreadComplete) {
                if (pStatus != NULL) {
                    pStatus->moveStatusToStop();
                    if (pJavaMethods != NULL) {
                        pJavaMethods->onCallJavaStop();
                    }
                }
            }
        }
    }
}

void SPlayer::destroy() {
    LOGD("SPlayer: destroy: ");

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


#pragma clang diagnostic pop