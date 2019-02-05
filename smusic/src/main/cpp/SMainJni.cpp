#include <jni.h>
#include "SLog.h"
#include "SPlayer.h"
#include "SJavaMethods.h"

JavaVM *_javaVM = NULL;
SPlayer *sPlayer = NULL;
SJavaMethods *sJavaMethods = NULL;

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGD("JNI_OnLoad");
    JNIEnv *jniEnv = NULL;
    if (vm->GetEnv(reinterpret_cast<void **>(&jniEnv), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    _javaVM = vm;
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {
    LOGD("JNI_OnUnload");
    _javaVM = NULL;
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_smusic_SMusic_nativeInit(JNIEnv *env, jobject instance) {

    LOGD("nativeInit");

    sJavaMethods = new SJavaMethods(_javaVM, env, instance);

    sPlayer = new SPlayer(_javaVM, sJavaMethods);

}
extern "C" JNIEXPORT void JNICALL Java_com_bzh_smusic_SMusic_nativeDestroy(JNIEnv *env, jobject instance) {

    LOGD("nativeDestroy");

    delete sJavaMethods;
    sJavaMethods = NULL;

    delete sPlayer;
    sPlayer = NULL;

}

extern "C" JNIEXPORT void JNICALL
Java_com_bzh_smusic_SMusic_nativePrepare(JNIEnv *env,
                                         jobject instance,
                                         jstring source_) {

    const char *source = env->GetStringUTFChars(source_, 0);

    // TODO

    env->ReleaseStringUTFChars(source_, source);
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_smusic_SMusic_nativeStart(JNIEnv *env, jobject instance) {

    // TODO

}