//
// Created by biezhihua on 2019/2/4.
//

#include "SPlayer.h"

SPlayer::SPlayer(JavaVM *pVm, SJavaMethods *pMethods) {
    pJavaVM = pVm;
    pJavaMethods = pMethods;
    pSFFmpeg = new SFFmpeg();
}

SPlayer::~SPlayer() {
    pJavaVM = NULL;
    pJavaMethods = NULL;
    pSource = NULL;
    delete pSFFmpeg;
    pSFFmpeg = NULL;
}

void SPlayer::setSource(string *url) {
    pSource = url;
    if (pSFFmpeg != NULL) {
        pSFFmpeg->setSource(url);
    }
}

void *prepareDecodeMediaInfo(void *data) {
    SPlayer *sPlayer = (SPlayer *) data;
    if (sPlayer != NULL && sPlayer->getSFFmpeg() != NULL) {
        SFFmpeg *pSFFmpeg = sPlayer->getSFFmpeg();
        if (pSFFmpeg != NULL) {
            int result = pSFFmpeg->decodeMediaInfo();
            if (result >= 0) {
                sPlayer->getSJavaMethods()->onCallJavaPrepared();
                LOGD("prepareDecodeMediaInfo end");
            }
        }
        pthread_exit(&sPlayer->prepareDecodeThread);
    }
}

void SPlayer::prepare() {
    pthread_create(&prepareDecodeThread, NULL, prepareDecodeMediaInfo, this);
}

void *startDecodeAudioFrame(void *data) {
    SPlayer *sPlayer = (SPlayer *) data;
    if (sPlayer != NULL) {
        SFFmpeg *pSFFmpeg = sPlayer->getSFFmpeg();
        if (pSFFmpeg != NULL) {
            int result = pSFFmpeg->decodeAudioFrame();
            if (result >= 0) {
                LOGD("startDecodeAudioFrame end");
            }
        }
        pthread_exit(&sPlayer->startDecodeAudioThread);
    }
}

void SPlayer::start() {
    pthread_create(&startDecodeAudioThread, NULL, startDecodeAudioFrame, this);
}

SFFmpeg *SPlayer::getSFFmpeg() {
    return pSFFmpeg;
}

SJavaMethods *SPlayer::getSJavaMethods() {
    return pJavaMethods;
}

