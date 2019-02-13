#include <jni.h>
#include <pthread.h>
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

extern "C" JNIEXPORT void JNICALL
Java_com_bzh_smusic_lib_SMusic_nativeSetSource(JNIEnv *env, jobject instance, jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);
    if (sPlayer != NULL) {
        sPlayer->setSource(new std::string(source));
    }
    env->ReleaseStringUTFChars(source_, source);
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_smusic_lib_SMusic_nativeCreate(JNIEnv *env, jobject instance) {

    LOGD("nativeInit");

    sJavaMethods = new SJavaMethods(_javaVM, env, instance);

    sPlayer = new SPlayer(_javaVM, env, instance, sJavaMethods);

}

extern "C" JNIEXPORT void JNICALL
Java_com_bzh_smusic_lib_SMusic_nativeStart(JNIEnv *env, jobject instance) {
    if (sPlayer != NULL) {
        sPlayer->start();
    }
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_smusic_lib_SMusic_nativePlay(JNIEnv *env, jobject instance) {
    if (sPlayer != NULL) {
        sPlayer->play();
    }
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_smusic_lib_SMusic_nativePause(JNIEnv *env, jobject instance) {
    if (sPlayer != NULL) {
        sPlayer->pause();
    }
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_smusic_lib_SMusic_nativeStop(JNIEnv *env, jobject instance) {
    if (sPlayer != NULL) {
        sPlayer->stop();
    }
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_smusic_lib_SMusic_nativeDestroy(JNIEnv *env, jobject instance) {

    LOGD("nativeRelease");

    delete sPlayer;
    sPlayer = NULL;

    delete sJavaMethods;
    sJavaMethods = NULL;

}
