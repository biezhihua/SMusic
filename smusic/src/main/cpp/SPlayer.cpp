//
// Created by biezhihua on 2019/2/4.
//

#include "SPlayer.h"

void *startDecodeAudioFrameCallback(void *data) {
    SPlayer *sPlayer = (SPlayer *) data;
    if (sPlayer != NULL) {
        SFFmpeg *pSFFmpeg = sPlayer->getSFFmpeg();
        SStatus *pPlayerStatus = sPlayer->getPlayerStatus();
        if (pSFFmpeg != NULL && pPlayerStatus != NULL) {
            while (!pPlayerStatus->isDestroy()) {
                int result = pSFFmpeg->decodeAudioFrame();
                if (result == SUCCESS) {

                } else if (result == ERROR_BREAK) {
                    LOGE("decodeMediaInfo error");
                    while (!pPlayerStatus->isDestroy()) {
                        SMedia *pAudio = pSFFmpeg->getAudio();
                        if (pAudio == NULL) {
                            break;
                        }
                        if (pAudio->getQueueSize() > 0) {
                            continue;
                        } else {
                            // pStatus->changeStateToExit();
                            break;
                        }
                    }
                }
            }
        }
        pthread_exit(&sPlayer->startDecodeAudioThread);
    }
    return NULL;
}

void *prepareDecodeMediaInfoCallback(void *data) {
    SPlayer *sPlayer = (SPlayer *) data;
    if (sPlayer != NULL && sPlayer->getSFFmpeg() != NULL) {
        SFFmpeg *pSFFmpeg = sPlayer->getSFFmpeg();
        if (pSFFmpeg != NULL) {
            int result = pSFFmpeg->decodeMediaInfo();
            if (result >= 0) {
                sPlayer->getSJavaMethods()->onCallJavaPrepared();
                LOGE("prepareDecodeMediaInfoCallback end");
            } else {
                LOGE("prepareDecodeMediaInfoCallback error");
            }
        }
        pthread_exit(&sPlayer->prepareDecodeThread);
    }
    return NULL;
}

void *playVideoCallback(void *data) {
    SPlayer *sPlayer = (SPlayer *) data;
    if (sPlayer != NULL && sPlayer->getSFFmpeg() != NULL) {
        SFFmpeg *pSFFmpeg = sPlayer->getSFFmpeg();
        SStatus *pPlayerStatus = sPlayer->getPlayerStatus();
        if (pPlayerStatus != NULL && pSFFmpeg != NULL) {
            while (!pPlayerStatus->isDestroy()) {
                int result = pSFFmpeg->resampleAudio();
                if (result == ERROR_BREAK) {
                    break;
                } else if (result == ERROR_CONTINUE) {
                    continue;
                } else if (result >= 0) {

                }
            }
        }
        pthread_exit(&sPlayer->playThread);
    }
    return NULL;
}

SPlayer::SPlayer(JavaVM *pVm, JNIEnv *env, jobject instance, SJavaMethods *pMethods) {
    pJavaVM = pVm;
    mainJniEnv = env;
    javaInstance = mainJniEnv->NewGlobalRef(instance);
    pJavaMethods = pMethods;

    pStatus = new SStatus();
    pSFFmpeg = new SFFmpeg();
    pOpenSLES = new SOpenSLES();

    if (pJavaMethods != NULL) {
        pJavaMethods->onCallJavaCreate();
    }
}

SPlayer::~SPlayer() {
    pSource = NULL;

//    delete pOpenSLES;
//    pOpenSLES = NULL;
//
//    delete pSFFmpeg;
//    pSFFmpeg = NULL;
//
//    delete pStatus;
//    pStatus = NULL;

    if (pJavaMethods != NULL) {
        pJavaMethods->onCallJavaDestroy();
    }

    pJavaMethods = NULL;
    mainJniEnv->DeleteGlobalRef(javaInstance);
    javaInstance = NULL;
    mainJniEnv = NULL;
    pJavaVM = NULL;
}

void SPlayer::setSource(string *url) {
    pSource = url;
    if (pSFFmpeg != NULL) {
        pSFFmpeg->setSource(url);
    }
}

void SPlayer::prepare() {
    pthread_create(&prepareDecodeThread, NULL, prepareDecodeMediaInfoCallback, this);
}

void SPlayer::start() {

    pthread_create(&startDecodeAudioThread, NULL, startDecodeAudioFrameCallback, this);

    pthread_create(&playThread, NULL, playVideoCallback, this);
}

SFFmpeg *SPlayer::getSFFmpeg() {
    return pSFFmpeg;
}

SJavaMethods *SPlayer::getSJavaMethods() {
    return pJavaMethods;
}


SStatus *SPlayer::getPlayerStatus() {
    return pStatus;
}

