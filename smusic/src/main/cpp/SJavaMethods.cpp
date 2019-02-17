//
// Created by biezhihua on 2019/2/4.
//

#include "SJavaMethods.h"

#include <pthread.h>

SJavaMethods::SJavaMethods(JavaVM *pVm, JNIEnv *pEnv, jobject pInstance) {
    javaVm = pVm;
    mainJniEnv = pEnv;
    javaInstance = mainJniEnv->NewGlobalRef(pInstance);

    jclass jClazz = mainJniEnv->GetObjectClass(javaInstance);
    if (jClazz != NULL) {
        idCreate = mainJniEnv->GetMethodID(jClazz, "onPlayerCreateFromNative", "()V");
        idStart = mainJniEnv->GetMethodID(jClazz, "onPlayerStartFromNative", "()V");
        idPlay = mainJniEnv->GetMethodID(jClazz, "onPlayerPlayFromNative", "()V");
        idPause = mainJniEnv->GetMethodID(jClazz, "onPlayerPauseFromNative", "()V");
        idStop = mainJniEnv->GetMethodID(jClazz, "onPlayerStopFromNative", "()V");
        idDestroy = mainJniEnv->GetMethodID(jClazz, "onPlayerDestroyFromNative", "()V");
        idTime = mainJniEnv->GetMethodID(jClazz, "onPlayerTimeFromNative", "(JJ)V");
    }
}

SJavaMethods::~SJavaMethods() {
    mainJniEnv->DeleteGlobalRef(javaInstance);
    javaInstance = NULL;
    mainJniEnv = NULL;
    javaVm = NULL;
}

bool SJavaMethods::isMainThread() {
    JNIEnv *jniEnv = NULL;
    if (javaVm != NULL) {
        jint res = javaVm->GetEnv(reinterpret_cast<void **>(&jniEnv), JNI_VERSION_1_6);
        bool result = res != JNI_EDETACHED;
        // LOGD("isMainThread %d %d", res, result);
        return result;
    }
    return false;
}

void SJavaMethods::onCallJavaCreate() {
    JNIEnv *jniEnv = tryLoadEnv();
    if (jniEnv != NULL) {
        jniEnv->CallVoidMethod(javaInstance, idCreate);
        tryUnLoadEnv();
    }
}

void SJavaMethods::onCallJavaStart() {
    JNIEnv *jniEnv = tryLoadEnv();
    if (jniEnv != NULL) {
        jniEnv->CallVoidMethod(javaInstance, idStart);
        tryUnLoadEnv();
    }
}

void SJavaMethods::onCallJavaPlay() {
    JNIEnv *jniEnv = tryLoadEnv();
    if (jniEnv != NULL) {
        jniEnv->CallVoidMethod(javaInstance, idPlay);
        tryUnLoadEnv();
    }
}

void SJavaMethods::onCallJavaPause() {
    JNIEnv *jniEnv = tryLoadEnv();
    if (jniEnv != NULL) {
        jniEnv->CallVoidMethod(javaInstance, idPause);
        tryUnLoadEnv();
    }
}

void SJavaMethods::onCallJavaStop() {
    JNIEnv *jniEnv = tryLoadEnv();
    if (jniEnv != NULL) {
        jniEnv->CallVoidMethod(javaInstance, idStop);
        tryUnLoadEnv();
    }
}

void SJavaMethods::onCallJavaDestroy() {
    JNIEnv *jniEnv = tryLoadEnv();
    if (jniEnv != NULL) {
        jniEnv->CallVoidMethod(javaInstance, idDestroy);
        tryUnLoadEnv();
    }
}

void SJavaMethods::onCallJavaTimeFromThread(long totalTimeMillis, long currentTimeMillis) {
    JNIEnv *jniEnv;
    if (javaVm->AttachCurrentThread(&jniEnv, 0) == JNI_OK) {
        jniEnv->CallVoidMethod(javaInstance, idTime, (jlong) totalTimeMillis, (jlong) currentTimeMillis);
        javaVm->DetachCurrentThread();
    }
}

void SJavaMethods::tryUnLoadEnv() {
    if (!isMainThread()) {
        javaVm->DetachCurrentThread();
    }
}

JNIEnv *SJavaMethods::tryLoadEnv() {
    JNIEnv *jniEnv = NULL;
    if (isMainThread()) {
        jniEnv = mainJniEnv;
    } else {
        int res = javaVm->AttachCurrentThread(&jniEnv, NULL);
        if (JNI_OK != res) {
            LOGE("Failed to AttachCurrentThread, ErrorCode = %d", res);
            jniEnv = NULL;
        }
    }
    return jniEnv;
}

