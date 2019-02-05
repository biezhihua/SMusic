//
// Created by biezhihua on 2019/2/4.
//

#include "SJavaMethods.h"

SJavaMethods::SJavaMethods(JavaVM *pVm, JNIEnv *pEnv, jobject pInstance) {
    javaVm = pVm;
    jniEnv = pEnv;
    javaInstance = jniEnv->NewGlobalRef(pInstance);
}

SJavaMethods::~SJavaMethods() {
    javaVm = NULL;
    jniEnv = NULL;
    javaInstance = NULL;
}
