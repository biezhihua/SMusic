//
// Created by biezhihua on 2019/2/4.
//

#ifndef SMUSIC_S_JAVA_METHODS_H
#define SMUSIC_S_JAVA_METHODS_H

#include <jni.h>
#include "SLog.h"

class SJavaMethods {
private:
    JavaVM *javaVm = NULL;
    JNIEnv *mainJniEnv = NULL;
    jobject javaInstance = NULL;

public:

    SJavaMethods(JavaVM *vm, JNIEnv *pEnv, jobject pJobject);

    ~SJavaMethods();

    void onCallJavaPrepared();

    bool isMainThread();
};


#endif //SMUSIC_S_JAVA_METHODS_H
