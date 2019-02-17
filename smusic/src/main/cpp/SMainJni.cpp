#include <jni.h>
#include <pthread.h>
#include "SLog.h"
#include "SPlayer.h"
#include "SJavaMethods.h"

JavaVM *_javaVM = NULL;
SPlayer *sPlayer = NULL;
SStatus *sStatus = NULL;
SJavaMethods *sJavaMethods = NULL;
pthread_t sDestroyThread;
bool sIsExiting = false;

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

    if (sPlayer == NULL) {

        if (sJavaMethods != NULL) {
            delete sJavaMethods;
            sJavaMethods = NULL;
        }

        sJavaMethods = new SJavaMethods(_javaVM, env, instance);

        sPlayer = new SPlayer(_javaVM, env, instance, sJavaMethods);

        sStatus = sPlayer->getPlayerStatus();

        if (sStatus != NULL) {
            sStatus->moveStatusToCreate();
        }
        if (sJavaMethods != NULL) {
            sJavaMethods->onCallJavaCreate();
        }
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_bzh_smusic_lib_SMusic_nativeStart(JNIEnv *env, jobject instance) {
    LOGD("nativeStart");
    if (sPlayer != NULL) {
        sPlayer->start();
    }
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_smusic_lib_SMusic_nativePlay(JNIEnv *env, jobject instance) {
    LOGD("nativePlay");
    if (sPlayer != NULL) {
        sPlayer->play();
    }
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_smusic_lib_SMusic_nativePause(JNIEnv *env, jobject instance) {
    LOGD("nativePause");
    if (sPlayer != NULL) {
        sPlayer->pause();
    }
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_smusic_lib_SMusic_nativeStop(JNIEnv *env, jobject instance) {
    LOGD("nativeStop");
    if (sPlayer != NULL) {
        sPlayer->stop();
    }
}

void *destroyCallBack(void *data) {
    if (data != NULL) {
        SStatus *status = (SStatus *) data;
        while (!status->isStop()) {
            continue;
        }
        status->moveStatusToPreDestroy();
        if (status->isPreDestroy()) {

            sStatus = NULL;

            delete sPlayer;
            sPlayer = NULL;
        }
    }
    sIsExiting = false;
    pthread_exit(&sDestroyThread);
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_smusic_lib_SMusic_nativeDestroy(JNIEnv *env, jobject instance) {
    LOGD("nativeRelease");
    if (sPlayer != NULL && sStatus != NULL && !sIsExiting) {
        if (sStatus->isCreate() || sStatus->isPreCreate() || sStatus->isSource()) {
            sStatus = NULL;

            delete sPlayer;
            sPlayer = NULL;

            delete sJavaMethods;
            sJavaMethods = NULL;
        } else {
            sPlayer->stop();
            pthread_create(&sDestroyThread, NULL, destroyCallBack, sStatus);
            sIsExiting = true;
        }
    }
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_bzh_smusic_lib_SMusic_nativeGetTotalTimeMillis(JNIEnv *env, jobject instance) {
    if (sPlayer != NULL) {
        SFFmpeg *ffmpeg = sPlayer->getSFFmpeg();
        if (ffmpeg != NULL) {
            return ffmpeg->getTotalTimeMillis();
        }
    }
    return 0;
}
extern "C" JNIEXPORT jlong JNICALL
Java_com_bzh_smusic_lib_SMusic_nativeGetCurrentTimeMillis(JNIEnv *env, jobject instance) {
    if (sPlayer != NULL) {
        SFFmpeg *ffmpeg = sPlayer->getSFFmpeg();
        if (ffmpeg != NULL) {
            return ffmpeg->getCurrentTimeMillis();
        }
    }
    return 0;
}

