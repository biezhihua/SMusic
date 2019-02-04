#include <jni.h>
#include <string>
#include <android/log.h>
#include <pthread.h>
#include <queue>
#include <unistd.h>
#include "SubThreadCallJavaContext.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#pragma clang diagnostic ignored "-Wmissing-noreturn"

extern "C" {
#include <libavformat/avformat.h>
}

#define TAG "smusic_jni"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__)

extern "C" JNIEXPORT jstring JNICALL
Java_com_bzh_smusic_Demo_stringFromJNI(JNIEnv *env, jobject /* this */) {
#if defined(__arm__)
#if defined(__ARM_ARCH_7A__)
#if defined(__ARM_NEON__)
#if defined(__ARM_PCS_VFP)
#define ABI "armeabi-v7a/NEON (hard-float)"
#else
#define ABI "armeabi-v7a/NEON"
#endif
#else
#if defined(__ARM_PCS_VFP)
#define ABI "armeabi-v7a (hard-float)"
#else
#define ABI "armeabi-v7a"
#endif
#endif
#else
#define ABI "armeabi"
#endif
#elif defined(__i386__)
#define ABI "x86"
#elif defined(__x86_64__)
#define ABI "x86_64"
#elif defined(__mips64)  /* mips64el-* toolchain defines __mips__ too */
#define ABI "mips64"
#elif defined(__mips__)
#define ABI "mips"
#elif defined(__aarch64__)
#define ABI "arm64-v8a"
#else
#define ABI "unknown"
#endif
    return env->NewStringUTF("Hello from JNI !  Compiled with ABI " ABI ".");
}

extern "C" JNIEXPORT void JNICALL Java_com_bzh_smusic_Demo_testFFmpeg(JNIEnv *env, jobject /* this */) {

    av_register_all();
    AVCodec *c_temp = av_codec_next(NULL);
    while (c_temp != NULL) {
        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO:
                LOGI("[Video]:%s", c_temp->name);
                break;
            case AVMEDIA_TYPE_AUDIO:
                LOGI("[Audio]:%s", c_temp->name);
                break;
            default:
                LOGI("[Other]:%s", c_temp->name);
                break;
        }
        c_temp = c_temp->next;
    }
}


pthread_t pthread;

void *normalThread(void *data) {
    LOGD("normal thread create");
    pthread_exit(&pthread);
}

extern "C" JNIEXPORT void JNICALL
Java_com_bzh_smusic_Demo_normalThread(JNIEnv *env, jobject /* this */) {
    pthread_create(&pthread, NULL, normalThread, NULL);
}


pthread_t product;
pthread_t custom;
pthread_mutex_t mutex;
pthread_cond_t cond;

std::queue<int> queue;

void *productThreadCallback(void *data) {
    LOGD("product thread create");
    while (1) {
        pthread_mutex_lock(&mutex);
        queue.push(1);
        LOGD("create one product, and notify custom %d", queue.size());
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        sleep(5);
    }
}

void *customThreadCallback(void *data) {
    LOGD("custom thread create");
    while (1) {
        pthread_mutex_lock(&mutex);
        if (queue.size() > 0) {
            queue.pop();
            LOGD("custom one product, residue %d", queue.size());
        } else {
            LOGD("custom thread not product, waiting product");
            pthread_cond_wait(&cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);
        usleep(1000 * 300);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_bzh_smusic_Demo_mutexThread(JNIEnv *env, jobject /* this */) {

    for (int i = 0; i < 10; i++) {
        queue.push(i);
    }

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_create(&product, NULL, productThreadCallback, NULL);
    pthread_create(&custom, NULL, customThreadCallback, NULL);
}

extern "C" JNIEXPORT void JNICALL
Java_com_bzh_smusic_Demo_mainThreadCallJava(JNIEnv *env, jobject obj) {
    LOGD("mainThreadCallJava");

    if (!env) {
        LOGE("Failed to retrieve env @ line %d", __LINE__);
        return;
    }

    jclass clazz = env->GetObjectClass(obj);

    if (!clazz) {
        LOGE("Failed to retrieve clazz @ line %d", __LINE__);
        return;
    }

    jmethodID methodId = env->GetMethodID(clazz, "onMainThreadError", "(ILjava/lang/String;)V");

    if (!methodId) {
        LOGE("Failed to retrieve methodId @ line %d", __LINE__);
        return;
    }

    jstring message = env->NewStringUTF("mainThreadCallJava");
    env->CallVoidMethod(obj, methodId, 100, message);
    env->DeleteLocalRef(message);
}

SubThreadCallJavaContext *subThreadCallJavaContext;

pthread_t subThreadCallJavaPThread;

void *subThreadCallJavaThread(void *data) {
    if (data == NULL) {
        LOGE("Failed to execute, data is null");
        return NULL;
    }

    SubThreadCallJavaContext *context = (SubThreadCallJavaContext *) data;
    JavaVM *javaVM = context->javaVM;
    JNIEnv *env;
    jint res = javaVM->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (res != JNI_OK) {
        res = javaVM->AttachCurrentThread(&env, NULL);
        if (JNI_OK != res) {
            LOGE("Failed to AttachCurrentThread, ErrorCode = %d", res);
            return NULL;
        }
    }

    jclass clazz = env->GetObjectClass(context->obj);

    if (!clazz) {
        LOGE("Failed to retrieve clazz @ line %d", __LINE__);
        return NULL;
    }

    jmethodID methodId = env->GetMethodID(clazz, "onSubThreadError", "(ILjava/lang/String;)V");

    if (!methodId) {
        LOGE("Failed to retrieve methodId @ line %d", __LINE__);
        return NULL;
    }

    jstring message = env->NewStringUTF("onSubThreadError");
    env->CallVoidMethod(context->obj, methodId, 100, message);
    env->DeleteLocalRef(message);

    javaVM->DetachCurrentThread();
    pthread_exit(&subThreadCallJavaPThread);
}

extern "C" JNIEXPORT void JNICALL
Java_com_bzh_smusic_Demo_subThreadCallJava(JNIEnv *env, jobject obj) {
    LOGD("subThreadCallJava");
    subThreadCallJavaContext->obj = env->NewGlobalRef(obj);
    pthread_create(&subThreadCallJavaPThread, NULL, subThreadCallJavaThread, subThreadCallJavaContext);
}

extern "C" JNIEXPORT void JNICALL
Java_com_bzh_smusic_Demo_mainThreadCallStaticJava(JNIEnv *env, jobject obj) {
    LOGD("mainThreadCallStaticJava");

    if (!env) {
        LOGE("Failed to retrieve env @ line %d", __LINE__);
        return;
    }

    jclass clazz = env->GetObjectClass(obj);

    if (!clazz) {
        LOGE("Failed to retrieve clazz @ line %d", __LINE__);
        return;
    }

    jmethodID methodId = env->GetStaticMethodID(clazz, "onMainThreadStaticError", "(ILjava/lang/String;)V");

    if (!methodId) {
        LOGE("Failed to retrieve methodId @ line %d", __LINE__);
        return;
    }

    jstring message = env->NewStringUTF("mainThreadCallStaticJava");
    env->CallStaticVoidMethod(clazz, methodId, 100, message);
    env->DeleteLocalRef(message);

}

pthread_t subThreadCallStaticJavaPThread;


void *subThreadCallStaticJavaThread(void *data) {
    if (data == NULL) {
        LOGE("Failed to execute, data is null");
        return NULL;
    }

    SubThreadCallJavaContext *context = (SubThreadCallJavaContext *) data;
    JavaVM *javaVM = context->javaVM;
    JNIEnv *env;
    jint res = javaVM->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (res != JNI_OK) {
        res = javaVM->AttachCurrentThread(&env, NULL);
        if (JNI_OK != res) {
            LOGE("Failed to AttachCurrentThread, ErrorCode = %d", res);
            return NULL;
        }
    }

    jclass clazz = env->GetObjectClass(context->obj);

    if (!clazz) {
        LOGE("Failed to retrieve clazz @ line %d", __LINE__);
        return NULL;
    }

    jmethodID methodId = env->GetStaticMethodID(clazz, "onSubThreadStaticError", "(ILjava/lang/String;)V");

    if (!methodId) {
        LOGE("Failed to retrieve methodId @ line %d", __LINE__);
        return NULL;
    }

    jstring message = env->NewStringUTF("onSubThreadStaticError");
    env->CallStaticVoidMethod(clazz, methodId, 100, message);
    env->DeleteLocalRef(message);

    javaVM->DetachCurrentThread();
    pthread_exit(&subThreadCallJavaPThread);
}

extern "C" JNIEXPORT void JNICALL
Java_com_bzh_smusic_Demo_subThreadCallStaticJava(JNIEnv *env, jobject obj) {
    LOGD("subThreadCallStaticJava");
    subThreadCallJavaContext->obj = env->NewGlobalRef(obj);
    pthread_create(&subThreadCallStaticJavaPThread, NULL, subThreadCallStaticJavaThread, subThreadCallJavaContext);
}

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGD("JNI_OnLoad");

    JNIEnv *env = NULL;

    // Check Current JNI Version
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;// JNI version not supported.
    }

    subThreadCallJavaContext = new SubThreadCallJavaContext();
    subThreadCallJavaContext->javaVM = vm;

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {
    LOGD("JNI_OnUnload");

    subThreadCallJavaContext->javaVM = NULL;
    delete subThreadCallJavaContext;
}

#pragma clang diagnostic pop