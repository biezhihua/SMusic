//
// Created by biezhihua on 2019/2/4.
//

#include "SPlayer.h"

SPlayer::SPlayer(JavaVM *pVm, SJavaMethods *pMethods) {
    this->javaVM = pVm;
    this->javaMethods = pMethods;
}

SPlayer::~SPlayer() {
    this->javaVM = NULL;
    this->javaMethods = NULL;
}
