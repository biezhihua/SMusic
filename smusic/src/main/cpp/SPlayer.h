//
// Created by biezhihua on 2019/2/4.
//

#ifndef SMUSIC_SMUSIC_PLAYER_H
#define SMUSIC_SMUSIC_PLAYER_H


#include <jni.h>
#include "SJavaMethods.h"
#include <string>
#include "SFFmpeg.h"

using namespace std;

class SPlayer {

private:
    const JavaVM *javaVM = NULL;

    const SJavaMethods *javaMethods = NULL;

    string *source = NULL;

public:

    SFFmpeg *sFFmpeg = NULL;

    pthread_t decodeThread;

public:
    SPlayer(JavaVM *pVm, SJavaMethods *pMethods);

    ~SPlayer();

    void setSource(string *url);

    void prepare();
};


#endif //SMUSIC_SMUSIC_PLAYER_H
