#ifndef SPLAYER_LOG_H
#define SPLAYER_LOG_H


#ifdef __ANDROID__

#include <android/log.h>

#define LOG_UNKNOWN     ANDROID_LOG_UNKNOWN
#define LOG_DEFAULT     ANDROID_LOG_DEFAULT

#define LOG_VERBOSE     ANDROID_LOG_VERBOSE
#define LOG_DEBUG       ANDROID_LOG_DEBUG
#define LOG_INFO        ANDROID_LOG_INFO
#define LOG_WARN        ANDROID_LOG_WARN
#define LOG_ERROR       ANDROID_LOG_ERROR
#define LOG_FATAL       ANDROID_LOG_FATAL
#define LOG_SILENT      ANDROID_LOG_SILENT

#define VLOG(level, TAG, ...)    ((void)__android_log_vprint(level, TAG, __VA_ARGS__))
#define ALOG(level, TAG, ...)    ((void)__android_log_print(level, TAG, __VA_ARGS__))

#else

#include <stdio.h>

#define LOG_UNKNOWN     0
#define LOG_DEFAULT     1

#define LOG_VERBOSE     2
#define LOG_DEBUG       3
#define LOG_INFO        4
#define LOG_WARN        5
#define LOG_ERROR       6
#define LOG_FATAL       7
#define LOG_SILENT      8

#define VLOG(level, TAG, ...)    ((void)vprintf(__VA_ARGS__))
#define ALOG(level, TAG, ...)    ((void)printf(__VA_ARGS__))

#endif

#define LOG_TAG "MEDIA"

#define VLOGV(...)  VLOG(LOG_VERBOSE,   LOG_TAG, __VA_ARGS__)
#define VLOGD(...)  VLOG(LOG_DEBUG,     LOG_TAG, __VA_ARGS__)
#define VLOGI(...)  VLOG(LOG_INFO,      LOG_TAG, __VA_ARGS__)
#define VLOGW(...)  VLOG(LOG_WARN,      LOG_TAG, __VA_ARGS__)
#define VLOGE(...)  VLOG(LOG_ERROR,     LOG_TAG, __VA_ARGS__)

#define ALOGV(...)  ALOG(LOG_VERBOSE,   LOG_TAG, __VA_ARGS__)
#define ALOGD(...)  ALOG(LOG_DEBUG,     LOG_TAG, __VA_ARGS__)
#define ALOGI(...)  ALOG(LOG_INFO,      LOG_TAG, __VA_ARGS__)
#define ALOGW(...)  ALOG(LOG_WARN,      LOG_TAG, __VA_ARGS__)
#define ALOGE(...)  ALOG(LOG_ERROR,     LOG_TAG, __VA_ARGS__)
#define LOG_ALWAYS_FATAL(...)   do { ALOGE(__VA_ARGS__); exit(1); } while (0)


#endif //SPLAYER_LOG_H
