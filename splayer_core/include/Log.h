#ifndef SPLAYER_LOG_H
#define SPLAYER_LOG_H

#define LOG_TAG "Media"

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

extern Mutex *logMutex;



// http://www.lihaoyi.com/post/BuildyourownCommandLinewithANSIescapecodes.html

#define _ALOGD(TAG, ...) do { \
logMutex->mutexLock();\
(void)printf("\x1B[37m"); \
(void)printf("%s-",LOG_TAG); \
(void)printf("\x1B[35;1m"); \
(void)printf("%-7s",Thread::getThreadNameById(pthread_self())); \
(void)printf("\033[0m"); \
(void)printf("-"); \
(void)printf("\x1B[32m"); \
(void)printf("%-13s",TAG); \
(void)printf("\033[0m"); \
(void)printf(__VA_ARGS__); \
(void)printf("\n"); \
logMutex->mutexUnLock(); \
} while (0)

#define _ALOGI(TAG, ...) do { \
logMutex->mutexLock();\
(void)printf("\x1B[37m"); \
(void)printf("%s-",LOG_TAG); \
(void)printf("\x1B[35;1m"); \
(void)printf("%-7s",Thread::getThreadNameById(pthread_self())); \
(void)printf("\033[0m"); \
(void)printf("-"); \
(void)printf("\x1B[33m"); \
(void)printf("%-13s",TAG); \
(void)printf("\033[0m: "); \
(void)printf(__VA_ARGS__); \
(void)printf("\n"); \
logMutex->mutexUnLock(); \
} while (0)

#define _ALOGE(TAG, ...) do { \
logMutex->mutexLock();\
(void)printf("\x1B[37m"); \
(void)printf("%s-",LOG_TAG); \
(void)printf("\x1B[35;1m"); \
(void)printf("%-7s",Thread::getThreadNameById(pthread_self())); \
(void)printf("\033[0m"); \
(void)printf("-"); \
(void)printf("\x1B[31m"); \
(void)printf("%-13s",TAG); \
(void)printf("\033[0m: "); \
(void)printf(__VA_ARGS__); \
(void)printf("\n"); \
logMutex->mutexUnLock(); \
} while (0)


#endif

#define ALOGD(TAG, ...)  _ALOGD(TAG, __VA_ARGS__)
#define ALOGI(TAG, ...)  _ALOGI(TAG, __VA_ARGS__)
#define ALOGE(TAG, ...)  _ALOGE(TAG, __VA_ARGS__)
#define ALOGV(TAG, ...)  _ALOGI(TAG, __VA_ARGS__)
#define ALOGW(TAG, ...)  _ALOGI(TAG, __VA_ARGS__)


#endif //SPLAYER_LOG_H
