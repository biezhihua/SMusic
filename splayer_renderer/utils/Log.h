#ifndef RENDERER_LOG_H
#define RENDERER_LOG_H

static bool RENDERER_DEBUG = false;

#ifdef __ANDROID__

#include <android/log.h>

#define _ALOGD(TAG, ...)    ((void)__android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__))
#define _ALOGI(TAG, ...)    ((void)__android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__))
#define _ALOGE(TAG, ...)    ((void)__android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__))
#define _ALOGW(TAG, ...)    ((void)__android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__))

#endif

#define ALOGD(TAG, ...)  _ALOGD(TAG, __VA_ARGS__)
#define ALOGI(TAG, ...)  _ALOGI(TAG, __VA_ARGS__)
#define ALOGE(TAG, ...)  _ALOGE(TAG, __VA_ARGS__)
#define ALOGV(TAG, ...)  _ALOGI(TAG, __VA_ARGS__)
#define ALOGW(TAG, ...)  _ALOGW(TAG, __VA_ARGS__)

#endif
