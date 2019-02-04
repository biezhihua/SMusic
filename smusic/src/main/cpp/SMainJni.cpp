#include <jni.h>
#include "SLog.h"

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGD("JNI_OnLoad");
    JNIEnv *jniEnv = NULL;
    if (vm->GetEnv(reinterpret_cast<void **>(&jniEnv), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {
    LOGD("JNI_OnUnload");
}

extern "C" JNIEXPORT void JNICALL
Java_com_bzh_smusic_SMusic_nativePrepare(JNIEnv *env,
                                         jobject instance,
                                         jstring source_) {

    const char *source = env->GetStringUTFChars(source_, 0);

    // TODO

    env->ReleaseStringUTFChars(source_, source);
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_smusic_SMusic_nativeStart(JNIEnv *env,
                                                                         jobject instance) {

    // TODO

}