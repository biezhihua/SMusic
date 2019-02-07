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

    JavaVM *pJavaVM = NULL;

    string *pSource = NULL;

    SJavaMethods *pJavaMethods = NULL;

    SFFmpeg *pSFFmpeg = NULL;

public:

    pthread_t prepareDecodeThread;

    pthread_t startDecodeAudioThread;

public:
    SPlayer(JavaVM *pVm, SJavaMethods *pMethods);

    ~SPlayer();

    void setSource(string *url);

    void prepare();

    SFFmpeg * getSFFmpeg();

    SJavaMethods* getSJavaMethods();

    void start();
};


#endif //SMUSIC_SMUSIC_PLAYER_H
