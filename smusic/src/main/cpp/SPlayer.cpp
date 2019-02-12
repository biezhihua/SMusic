//
// Created by biezhihua on 2019/2/4.
//

#include "SPlayer.h"

void *startDecodeAudioFrameCallback(void *data) {
    SPlayer *sPlayer = (SPlayer *) data;
    if (sPlayer != NULL) {
        SFFmpeg *pSFFmpeg = sPlayer->getSFFmpeg();
        SPlayerStatus *pPlayerStatus = sPlayer->getPlayerStatus();
        if (pSFFmpeg != NULL && pPlayerStatus != NULL) {
            while (!pPlayerStatus->isExit()) {
                int result = pSFFmpeg->decodeAudioFrame();
                if (result == SUCCESS) {

                } else if (result == ERROR_BREAK) {
                    LOGE("decodeMediaInfo error");
                    while (!pPlayerStatus->isExit()) {
                        SMedia *pAudio = pSFFmpeg->getAudio();
                        if (pAudio == NULL) {
                            break;
                        }
                        if (pAudio->getQueueSize() > 0) {
                            continue;
                        } else {
                            pPlayerStatus->changeStateToExit();
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
        SPlayerStatus *pPlayerStatus = sPlayer->getPlayerStatus();
        if (pPlayerStatus != NULL && pSFFmpeg != NULL) {
            while (!pPlayerStatus->isExit()) {
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

SPlayer::SPlayer(JavaVM *pVm, SJavaMethods *pMethods) {
    pJavaVM = pVm;
    pJavaMethods = pMethods;
    pSFFmpeg = new SFFmpeg();
    pPlayerStatus = new SPlayerStatus();
}

SPlayer::~SPlayer() {
    pJavaVM = NULL;
    pJavaMethods = NULL;
    pSource = NULL;
    delete pSFFmpeg;
    pSFFmpeg = NULL;
    delete pPlayerStatus;
    pPlayerStatus = NULL;
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


SPlayerStatus *SPlayer::getPlayerStatus() {
    return pPlayerStatus;
}

