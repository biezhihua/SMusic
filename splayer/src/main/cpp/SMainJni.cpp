#include <jni.h>
#include <pthread.h>
#include <SPlayer.h>
#include <SStatus.h>
#include <SFFmpeg.h>
#include <SOpenSLES.h>
#include "splayer/main/SLog.h"
#include "splayer/main/SJavaMethods.h"

#include "splayer/android/AndroidMediaPlayer.h"
#include "splayer/media/MediaPlayer.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

JavaVM *sJavaVM = nullptr;
SPlayer *sPlayer = nullptr;
SStatus *sStatus = nullptr;
SJavaMethods *sJavaMethods = nullptr;
MediaPlayer *mediaPlayer = nullptr;
#define JNI_TAG "Native_MainJni"

// Destroy Instance
pthread_t sDestroyThread;
bool sIsExiting = false;

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGD(JNI_TAG, "JNI_OnLoad");
    JNIEnv *jniEnv = nullptr;
    if (vm->GetEnv(reinterpret_cast<void **>(&jniEnv), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    sJavaVM = vm;
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {
    LOGD(JNI_TAG, "JNI_OnUnload");
    sJavaVM = nullptr;
}

extern "C" JNIEXPORT void JNICALL
Java_com_bzh_splayer_lib_SPlayer_nativeSetSource(JNIEnv *env, jobject instance, jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);
    if (sPlayer != nullptr) {
        sPlayer->setSource(new std::string(source));
    }
    env->ReleaseStringUTFChars(source_, source);
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_splayer_lib_SPlayer_nativeCreate(JNIEnv *env, jobject instance) {

    LOGD(JNI_TAG, "nativeInit");

    if (sPlayer == nullptr) {

        if (sJavaMethods != nullptr) {
            delete sJavaMethods;
            sJavaMethods = nullptr;
        }

        sJavaMethods = new SJavaMethods(sJavaVM, env, instance);

        sPlayer = new SPlayer(sJavaVM, env, instance, sJavaMethods);

        sStatus = sPlayer->getPlayerStatus();

        if (sStatus != nullptr) {
            sStatus->moveStatusToCreate();
        }
        if (sJavaMethods != nullptr) {
            sJavaMethods->onCallJavaCreate();
        }
    }

    mediaPlayer = ((MediaPlayer *) new AndroidMediaPlayer());
    mediaPlayer->create();
}



extern "C" JNIEXPORT void JNICALL
Java_com_bzh_splayer_lib_SPlayer_nativeStart(JNIEnv *env, jobject instance) {
    LOGD(JNI_TAG, "nativeStart");
    if (sPlayer != nullptr) {
        sPlayer->start();
    }
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_splayer_lib_SPlayer_nativePlay(JNIEnv *env, jobject instance) {
    LOGD(JNI_TAG, "nativePlay");
    if (sPlayer != nullptr) {
        sPlayer->play();
    }
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_splayer_lib_SPlayer_nativePause(JNIEnv *env, jobject instance) {
    LOGD(JNI_TAG, "nativePause");
    if (sPlayer != nullptr) {
        sPlayer->pause();
    }
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_splayer_lib_SPlayer_nativeStop(JNIEnv *env, jobject instance) {
    LOGD(JNI_TAG, "nativeStop");
    if (sPlayer != nullptr) {
        sPlayer->stop();
    }
}

void *destroyCallBack(void *data) {
    if (data != nullptr) {
        SStatus *status = (SStatus *) data;
        while (!status->isStop()) {
            continue;
        }
        status->moveStatusToPreDestroy();
        if (status->isPreDestroy()) {
            sStatus = nullptr;
            delete sPlayer;
            sPlayer = nullptr;
        }
    }
    sIsExiting = false;
    pthread_exit(&sDestroyThread);
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_splayer_lib_SPlayer_nativeDestroy(JNIEnv *env, jobject instance) {
    LOGD(JNI_TAG, "nativeRelease");
    if (sPlayer != nullptr && sStatus != nullptr && !sIsExiting) {
        if (sStatus->isCreate() || sStatus->isPreCreate() || sStatus->isSource() || sStatus->isComplete()) {
            sStatus = nullptr;

            delete sPlayer;
            sPlayer = nullptr;

            delete sJavaMethods;
            sJavaMethods = nullptr;
        } else {
            sPlayer->stop();
            pthread_create(&sDestroyThread, nullptr, destroyCallBack, sStatus);
            sIsExiting = true;
        }
    }
    delete mediaPlayer;
    mediaPlayer = nullptr;
}

extern "C" JNIEXPORT jint JNICALL Java_com_bzh_splayer_lib_SPlayer_nativeGetTotalTimeMillis(JNIEnv *env,
                                                                                            jobject instance) {
    if (sPlayer != nullptr) {
        SFFmpeg *ffmpeg = sPlayer->getFFmpeg();
        if (ffmpeg != nullptr) {
            return (int) ffmpeg->getTotalTimeMillis();
        }
    }
    return 0;
}

extern "C" JNIEXPORT jint JNICALL Java_com_bzh_splayer_lib_SPlayer_nativeGetCurrentTimeMillis(JNIEnv *env,
                                                                                              jobject instance) {
    if (sPlayer != nullptr) {
        SFFmpeg *ffmpeg = sPlayer->getFFmpeg();
        if (ffmpeg != nullptr) {
            return (int) ffmpeg->getCurrentTimeMillis();
        }
    }
    return 0;
}

extern "C" JNIEXPORT jint JNICALL Java_com_bzh_splayer_lib_SPlayer_nativeGetCurrentVolumePercent(JNIEnv *env,
                                                                                                 jobject instance) {
    if (sPlayer != nullptr) {
        SOpenSLES *openSLES = sPlayer->getSOpenSLES();
        if (openSLES != nullptr) {
            return (int) openSLES->getCurrentVolumePercent();
        }
    }
    return 0;
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_splayer_lib_SPlayer_nativeSeek(JNIEnv *env,
                                                                              jobject instance,
                                                                              jint millis) {
    LOGD(JNI_TAG, "nativeSeek %d", (int) millis);
    if (sPlayer != nullptr) {
        sPlayer->seek((int64_t) millis);
    }
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_splayer_lib_SPlayer_nativeVolume(JNIEnv *env,
                                                                                jobject instance,
                                                                                jint percent) {
    LOGD(JNI_TAG, "nativeVolume %d", (int) percent);
    if (sPlayer != nullptr) {
        sPlayer->volume((int) percent);
    }
}


extern "C" JNIEXPORT void JNICALL Java_com_bzh_splayer_lib_SPlayer_nativeMute(JNIEnv *env,
                                                                              jobject instance,
                                                                              jint mute) {
    LOGD(JNI_TAG, "nativeMute %d", (int) mute);
    if (sPlayer != nullptr) {
        sPlayer->mute((int) mute);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_bzh_splayer_lib_SPlayer_nativeSpeed(JNIEnv *env, jobject instance, jdouble speed) {
    LOGD(JNI_TAG, "nativeSpeed %f", (double) speed);
    if (sPlayer != nullptr) {
        sPlayer->speed((double) speed);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_bzh_splayer_lib_SPlayer_nativePitch(JNIEnv *env, jobject instance, jdouble pitch) {
    LOGD(JNI_TAG, "nativePitch %f", (double) pitch);
    if (sPlayer != nullptr) {
        sPlayer->pitch((double) pitch);
    }
}


extern "C" JNIEXPORT jdouble JNICALL Java_com_bzh_splayer_lib_SPlayer_nativeGetCurrentSpeed(JNIEnv *env,
                                                                                            jobject instance) {
    if (sPlayer != nullptr) {
        SOpenSLES *openSLES = sPlayer->getSOpenSLES();
        if (openSLES != nullptr) {
            return (int) openSLES->getSoundSpeed();
        }
    }
    return 0;
}


extern "C" JNIEXPORT jdouble JNICALL Java_com_bzh_splayer_lib_SPlayer_nativeGetCurrentPitch(JNIEnv *env,
                                                                                            jobject instance) {
    if (sPlayer != nullptr) {
        SOpenSLES *openSLES = sPlayer->getSOpenSLES();
        if (openSLES != nullptr) {
            return (int) openSLES->getSoundPitch();
        }
    }
    return 0;
}
#pragma clang diagnostic pop