#ifndef SPLAYER_S_JAVA_METHODS_H
#define SPLAYER_S_JAVA_METHODS_H

#include <jni.h>
#include <string>
#include "SLog.h"
#include <pthread.h>

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
    jmethodID idRender;
    jmethodID idIsSupport;

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

    void onCallJavaRenderYUVFromThread(int width, int height, uint8_t *y, uint8_t *u, uint8_t *v);

    bool isMainThread();

    bool isSupportMediaCodec(const char * codecName);

};


#endif //SPLAYER_S_JAVA_METHODS_H
