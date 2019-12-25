#ifndef ENGINE_LOG_H
#define ENGINE_LOG_H

#define LOG_TAG "Media"

static bool ENGINE_DEBUG = true;

#ifdef __ANDROID__

#include <android/log.h>

#define _ALOGD(TAG, ...)    ((void)__android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__))
#define _ALOGI(TAG, ...)    ((void)__android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__))
#define _ALOGE(TAG, ...)    ((void)__android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__))
#define _ALOGW(TAG, ...)    ((void)__android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__))

#elif SPLAYER_COMMAND

#include <pthread.h>
#include "Mutex.h"
#include <stdio.h>
#include "Thread.h"

#define LOG_UNKNOWN     0
#define LOG_DEFAULT     1

#define LOG_VERBOSE     2
#define LOG_DEBUG       3
#define LOG_INFO        4
#define LOG_WARN        5
#define LOG_ERROR       6
#define LOG_FATAL       7
#define LOG_SILENT      8

extern Mutex *LOG_MUTEX;

// http://www.lihaoyi.com/post/BuildyourownCommandLinewithANSIescapecodes.html

#define _ALOGD(TAG, ...) do { \
LOG_MUTEX->lock();\
(void)printf("\x1B[37m"); \
(void)printf("%s ",LOG_TAG); \
(void)printf("\033[0m"); \
(void)printf(" "); \
(void)printf("\x1B[32m"); \
(void)printf("%-15s",TAG); \
(void)printf("\033[0m: "); \
(void)printf(__VA_ARGS__); \
(void)printf("\n"); \
LOG_MUTEX->unlock(); \
} while (0)

#define _ALOGI(TAG, ...) do { \
LOG_MUTEX->lock();\
//(void)printf("\x1B[37m"); \
(void)printf("%s ",LOG_TAG); \
(void)printf("\033[0m"); \
(void)printf(" "); \
(void)printf("\x1B[30;1m"); \
(void)printf("%-15s",TAG); \
(void)printf("\033[0m: "); \
(void)printf(__VA_ARGS__); \
(void)printf("\n"); \
LOG_MUTEX->unlock(); \
} while (0)

#define _ALOGE(TAG, ...) do { \
LOG_MUTEX->lock();\
(void)printf("\x1B[37m"); \
(void)printf("%s ",LOG_TAG); \
(void)printf("\033[0m"); \
(void)printf(" "); \
(void)printf("\x1B[31m"); \
(void)printf("%-15s",TAG); \
(void)printf("\033[0m: "); \
(void)printf(__VA_ARGS__); \
(void)printf("\n"); \
LOG_MUTEX->unlock(); \
} while (0)

#define _ALOGW(TAG, ...) do { \
LOG_MUTEX->lock();\
(void)printf("\x1B[37m"); \
(void)printf("%s ",LOG_TAG); \
(void)printf("\033[0m"); \
(void)printf(" "); \
(void)printf("\x1B[33m"); \
(void)printf("%-15s",TAG); \
(void)printf("\033[0m: "); \
(void)printf(__VA_ARGS__); \
(void)printf("\n"); \
LOG_MUTEX->unlock(); \
} while (0)

#elif __APPLE__

#include <sys/syslog.h>

#define _ALOGD(TAG, ...) (void)syslog(LOG_DEBUG, __VA_ARGS__);
#define _ALOGI(TAG, ...) (void)syslog(LOG_INFO,__VA_ARGS__);
#define _ALOGE(TAG, ...) (void)syslog(LOG_ERR,__VA_ARGS__);
#define _ALOGV(TAG, ...) (void)syslog(LOG_NOTICE,__VA_ARGS__);
#define _ALOGW(TAG, ...) (void)syslog(LOG_WARNING,__VA_ARGS__);

#else

#define _ALOGD(TAG, ...) (void)printf(__VA_ARGS__);
#define _ALOGI(TAG, ...) (void)printf(__VA_ARGS__);
#define _ALOGE(TAG, ...) (void)printf(__VA_ARGS__);
#define _ALOGV(TAG, ...) (void)printf(__VA_ARGS__);
#define _ALOGW(TAG, ...) (void)printf(__VA_ARGS__);

#endif

#define ALOGD(TAG, ...)  _ALOGD(TAG, __VA_ARGS__)
#define ALOGI(TAG, ...)  _ALOGI(TAG, __VA_ARGS__)
#define ALOGE(TAG, ...)  _ALOGE(TAG, __VA_ARGS__)
#define ALOGV(TAG, ...)  _ALOGI(TAG, __VA_ARGS__)
#define ALOGW(TAG, ...)  _ALOGW(TAG, __VA_ARGS__)

#endif
