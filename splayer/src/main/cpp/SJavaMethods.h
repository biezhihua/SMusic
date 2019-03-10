//
// Created by biezhihua on 2019/2/4.
//

#ifndef SPLAYER_S_JAVA_METHODS_H
#define SPLAYER_S_JAVA_METHODS_H

#include <jni.h>
#include <string>
#include "SLog.h"

class SJavaMethods {
private:
    JavaVM *javaVm = NULL;
    JNIEnv *mainJniEnv = NULL;
    jobject javaInstance = NULL;

    jmethodID idCreate;
    jmethodID idStart;
    jmethodID idPlay;
    jmethodID idPause;
    jmethodID idStop;
    jmethodID idDestroy;
    jmethodID idTime;
    jmethodID idError;
    jmethodID idComplete;
    jmethodID idLoad;

    JNIEnv *tryLoadEnv();

    void tryUnLoadEnv();

public:

    SJavaMethods(JavaVM *vm, JNIEnv *pEnv, jobject pJObject);

    ~SJavaMethods();

    void onCallJavaCreate();

    void onCallJavaStart();

    void onCallJavaPlay();

    void onCallJavaPause();

    void onCallJavaStop();

    void onCallJavaDestroy();

    void onCallJavaTimeFromThread(int totalTimeMillis, int currentTimeMillis);

    void onCallJavaError(int code, const char *message);

    void onCallJavaComplete();

    void onCallJavaLoadState(bool loadState);

    bool isMainThread();

};


#endif //SPLAYER_S_JAVA_METHODS_H
