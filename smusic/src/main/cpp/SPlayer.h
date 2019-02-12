//
// Created by biezhihua on 2019/2/4.
//

#ifndef SMUSIC_SMUSIC_PLAYER_H
#define SMUSIC_SMUSIC_PLAYER_H


#include <jni.h>
#include <string>
#include "SJavaMethods.h"
#include "SFFmpeg.h"
#include "SPlayerStatus.h"
#include "SError.h"

class SPlayer {

private:

    JavaVM *pJavaVM = NULL;

    string *pSource = NULL;

    SJavaMethods *pJavaMethods = NULL;

    SFFmpeg *pSFFmpeg = NULL;

    SPlayerStatus *pPlayerStatus = NULL;

public:

    pthread_t prepareDecodeThread;

    pthread_t startDecodeAudioThread;

    pthread_t playThread;

public:
    SPlayer(JavaVM *pVm, SJavaMethods *pMethods);

    ~SPlayer();

    void setSource(string *url);

    void prepare();

    void start();

    SFFmpeg *getSFFmpeg();

    SJavaMethods *getSJavaMethods();

    SPlayerStatus *getPlayerStatus();
};


#endif //SMUSIC_SMUSIC_PLAYER_H
