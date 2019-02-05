//
// Created by biezhihua on 2019/2/4.
//

#ifndef SMUSIC_S_JAVA_METHODS_H
#define SMUSIC_S_JAVA_METHODS_H

#include <jni.h>

class SJavaMethods {
private:
    JavaVM *javaVm = NULL;
    JNIEnv *jniEnv = NULL;
    jobject javaInstance = NULL;

public:

    SJavaMethods(JavaVM *vm, JNIEnv *pEnv, jobject pJobject);

    ~SJavaMethods();
};


#endif //SMUSIC_S_JAVA_METHODS_H
