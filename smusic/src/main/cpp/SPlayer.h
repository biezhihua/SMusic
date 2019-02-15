//
// Created by biezhihua on 2019/2/4.
//

#ifndef SMUSIC_SMUSIC_PLAYER_H
#define SMUSIC_SMUSIC_PLAYER_H


#include <jni.h>
#include <string>
#include "SJavaMethods.h"
#include "SFFmpeg.h"
#include "SStatus.h"
#include "SError.h"
#include "SOpenSLES.h"

class SPlayer {

private:

    JavaVM *pJavaVM = NULL;

    JNIEnv *mainJniEnv = NULL;

    jobject javaInstance = NULL;

    string *pSource = NULL;

    SJavaMethods *pJavaMethods = NULL;

    SFFmpeg *pFFmpeg = NULL;

    SStatus *pStatus = NULL;

    SOpenSLES *pOpenSLES = NULL;

public:

    pthread_t startDecodeMediaInfoThread;

    pthread_t startDecodeAudioThread;

    pthread_t playAudioThread;

private:
    void create();

    void destroy();

public:
    SPlayer(JavaVM *pVm, JNIEnv *env, jobject instance, SJavaMethods *pMethods);

    ~SPlayer();

    void setSource(string *url);

    void start();

    void play();

    void pause();

    void stop();

    SFFmpeg *getSFFmpeg();

    SJavaMethods *getSJavaMethods();

    SStatus *getPlayerStatus();

    SOpenSLES * getSOpenSLES();

};


#endif //SMUSIC_SMUSIC_PLAYER_H
