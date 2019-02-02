#include <jni.h>
#include <string>
#include <android/log.h>
#include <pthread.h>
#include <queue>
#include <unistd.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#pragma clang diagnostic ignored "-Wmissing-noreturn"

extern "C" {
#include <libavformat/avformat.h>
}

#define TAG "smusic_jni" // 这个是自定义的LOG的标识
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型

extern "C" JNIEXPORT jstring JNICALL
Java_com_bzh_smusic_Demo_stringFromJNI(JNIEnv *env, jobject /* this */) {
    std::string hello = "Hello from FFmpeg C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_bzh_smusic_Demo_testFFmpeg(JNIEnv *env, jobject /* this */) {

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


#pragma clang diagnostic pop