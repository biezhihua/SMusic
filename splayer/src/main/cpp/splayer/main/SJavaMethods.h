#ifndef SPLAYER_S_JAVA_METHODS_H
#define SPLAYER_S_JAVA_METHODS_H

#include <jni.h>
#include <string>
#include "SLog.h"
#include <pthread.h>

class SJavaMethods {
private:
    JavaVM *javaVm = nullptr;
    JNIEnv *mainJniEnv = nullptr;
    jobject javaInstance = nullptr;

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
    jmethodID idInitMediaCodec;
    jmethodID idMediaCodecDecodeAvPacke;

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

    bool isSupportMediaCodec(const char *codecName);

    void
    onCallJavaInitMediaCodec(const char *codecName, int width, int height, int cds0Size, uint8_t *cds0, int cds1Size,
                             uint8_t *cds1);

    void onCallJavaMediaCodecDecodeAvPacket(int dataSize, uint8_t *data);

};


#endif //SPLAYER_S_JAVA_METHODS_H
