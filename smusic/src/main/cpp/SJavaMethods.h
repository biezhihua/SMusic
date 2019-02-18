//
// Created by biezhihua on 2019/2/4.
//

#ifndef SMUSIC_S_JAVA_METHODS_H
#define SMUSIC_S_JAVA_METHODS_H

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

    JNIEnv *tryLoadEnv();

    void tryUnLoadEnv();

public:

    SJavaMethods(JavaVM *vm, JNIEnv *pEnv, jobject pJobject);

    ~SJavaMethods();

    void onCallJavaCreate();

    void onCallJavaStart();

    void onCallJavaPlay();

    void onCallJavaPause();

    void onCallJavaStop();

    void onCallJavaDestroy();

    void onCallJavaTimeFromThread(long totalTimeMillis, long currentTimeMillis);

    void onCallJavaError(int code, const char *message);

    void onCallJavaComplete();

    bool isMainThread();

};


#endif //SMUSIC_S_JAVA_METHODS_H
