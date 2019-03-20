#ifndef SPLAYER_S_LOG_H
#define SPLAYER_S_LOG_H

#include <android/log.h>

#define LOGD(TAG, ...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__)
#define LOGI(TAG, ...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__)
#define LOGW(TAG, ...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__)
#define LOGE(TAG, ...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__)
#define LOGF(TAG, ...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__)

#endif //SPLAYER_S_LOG_H
