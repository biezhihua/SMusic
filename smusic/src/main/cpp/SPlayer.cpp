//
// Created by biezhihua on 2019/2/4.
//

#include "SPlayer.h"


SPlayer::SPlayer(JavaVM *pVm, SJavaMethods *pMethods) {
    javaVM = pVm;
    javaMethods = pMethods;
    sFFmpeg = new SFFmpeg();
}

SPlayer::~SPlayer() {
    javaVM = NULL;
    javaMethods = NULL;
    source = NULL;
    delete sFFmpeg;
    sFFmpeg = NULL;
}

void SPlayer::setSource(string *url) {
    source = url;
    sFFmpeg->setSource(url);
}

void *decodeFFmpeg(void *data) {
    SPlayer *sPlayer = (SPlayer *) data;
    if (sPlayer != NULL && sPlayer->sFFmpeg != NULL) {
        sPlayer->sFFmpeg->decodeFFmpeg();
        pthread_exit(&sPlayer->decodeThread);
    }
}


void SPlayer::prepare() {
    pthread_create(&decodeThread, NULL, decodeFFmpeg, this);
}
