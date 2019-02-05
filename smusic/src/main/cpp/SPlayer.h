//
// Created by biezhihua on 2019/2/4.
//

#ifndef SMUSIC_SMUSIC_PLAYER_H
#define SMUSIC_SMUSIC_PLAYER_H


#include <jni.h>
#include "SJavaMethods.h"

class SPlayer {

private:
    JavaVM *javaVM = NULL;
    SJavaMethods *javaMethods = NULL;

public:
    SPlayer(JavaVM *vm, SJavaMethods *pMethods);

    ~SPlayer();
};


#endif //SMUSIC_SMUSIC_PLAYER_H
