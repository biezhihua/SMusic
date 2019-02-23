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
        idTime = mainJniEnv->GetMethodID(jClazz, "onPlayerTimeFromNative", "(II)V");
        idError = mainJniEnv->GetMethodID(jClazz, "onPlayerErrorFromNative", "(ILjava/lang/String;)V");
        idComplete = mainJniEnv->GetMethodID(jClazz, "onPlayerCompleteFromNative", "()V");
        idLoad = mainJniEnv->GetMethodID(jClazz, "onPlayerLoadStateFromNative", "(Z)V");
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
    LOGD("SJavaMethods:onCallJavaCreate");
    JNIEnv *jniEnv = tryLoadEnv();
    if (jniEnv != NULL) {
        jniEnv->CallVoidMethod(javaInstance, idCreate);
        tryUnLoadEnv();
    }
}

void SJavaMethods::onCallJavaStart() {
    LOGD("SJavaMethods:onCallJavaStart");
    JNIEnv *jniEnv = tryLoadEnv();
    if (jniEnv != NULL) {
        jniEnv->CallVoidMethod(javaInstance, idStart);
        tryUnLoadEnv();
    }
}

void SJavaMethods::onCallJavaPlay() {
    LOGD("SJavaMethods:onCallJavaPlay");
    JNIEnv *jniEnv = tryLoadEnv();
    if (jniEnv != NULL) {
        jniEnv->CallVoidMethod(javaInstance, idPlay);
        tryUnLoadEnv();
    }
}

void SJavaMethods::onCallJavaPause() {
    LOGD("SJavaMethods:onCallJavaPause");
    JNIEnv *jniEnv = tryLoadEnv();
    if (jniEnv != NULL) {
        jniEnv->CallVoidMethod(javaInstance, idPause);
        tryUnLoadEnv();
    }
}

void SJavaMethods::onCallJavaStop() {
    LOGD("SJavaMethods:onCallJavaStop");
    JNIEnv *jniEnv = tryLoadEnv();
    if (jniEnv != NULL) {
        jniEnv->CallVoidMethod(javaInstance, idStop);
        tryUnLoadEnv();
    }
}

void SJavaMethods::onCallJavaDestroy() {
    LOGD("SJavaMethods:onCallJavaDestroy");
    JNIEnv *jniEnv = tryLoadEnv();
    if (jniEnv != NULL) {
        jniEnv->CallVoidMethod(javaInstance, idDestroy);
        tryUnLoadEnv();
    }
}

void SJavaMethods::onCallJavaTimeFromThread(int totalTimeMillis, int currentTimeMillis) {
    LOGD("SJavaMethods:onCallJavaTime: %d %d", totalTimeMillis, currentTimeMillis);
    JNIEnv *jniEnv;
    if (javaVm->AttachCurrentThread(&jniEnv, 0) == JNI_OK) {
        jniEnv->CallVoidMethod(javaInstance, idTime, (jint) totalTimeMillis, (jint) currentTimeMillis);
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

void SJavaMethods::onCallJavaError(int code, const char *message) {
    LOGD("SJavaMethods:onCallJavaError: %s", message);
    JNIEnv *jniEnv;
    if (javaVm->AttachCurrentThread(&jniEnv, 0) == JNI_OK) {
        jstring jMessage = jniEnv->NewStringUTF(message);
        jniEnv->CallVoidMethod(javaInstance, idError, code, jMessage);
        jniEnv->DeleteLocalRef(jMessage);
        javaVm->DetachCurrentThread();
    }
}

void SJavaMethods::onCallJavaComplete() {
    LOGD("SJavaMethods:onCallJavaComplete");
    JNIEnv *jniEnv = tryLoadEnv();
    if (jniEnv != NULL) {
        jniEnv->CallVoidMethod(javaInstance, idComplete);
        tryUnLoadEnv();
    }
}

void SJavaMethods::onCallJavaLoadState(bool loadState) {
    JNIEnv *jniEnv;
    LOGD("SJavaMethods:onCallJavaLoadState %d", loadState);
    if (javaVm->AttachCurrentThread(&jniEnv, 0) == JNI_OK) {
        jniEnv->CallVoidMethod(javaInstance, idLoad, loadState);
        javaVm->DetachCurrentThread();
    }
}

