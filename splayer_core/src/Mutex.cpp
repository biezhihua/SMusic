#include "Mutex.h"

int Mutex::mutexLock() {
    return pthread_mutex_lock(&mutex);
}

int Mutex::mutexUnLock() {
    return pthread_mutex_unlock(&mutex);
}

int Mutex::condSignal() {
    return pthread_cond_signal(&cond);
}

int Mutex::condBroadcast() {
    return pthread_cond_broadcast(&cond);
}

int Mutex::condWaitTimeout(uint32_t ms) {
    int ret;
    struct timeval delta;
    struct timespec abstime;

    gettimeofday(&delta, nullptr);

    abstime.tv_sec = static_cast<time_t>(delta.tv_sec + (ms / 1000));
    abstime.tv_nsec = static_cast<long>((delta.tv_usec + (ms % 1000) * 1000) * 1000);

    if (abstime.tv_nsec > 1000000000) {
        abstime.tv_sec += 1;
        abstime.tv_nsec -= 1000000000;
    }

    while (true) {
        ret = pthread_cond_timedwait(&cond, &mutex, &abstime);
        if (ret == 0)
            return 0;
        else if (ret == EINTR)
            continue;
        else if (ret == ETIMEDOUT)
            return MUTEX_TIMEDOUT;
        else
            break;
    }

    return -1;
}

int Mutex::condWait() {
    return pthread_cond_wait(&cond, &mutex);
}

Mutex::~Mutex() {
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}

Mutex::Mutex() {
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);
}

int Mutex::condWaitTimeout(Mutex *other, uint32_t ms) {
    if (!other) {
        return -1;
    }

    int ret;
    struct timeval delta;
    struct timespec abstime;

    gettimeofday(&delta, nullptr);

    abstime.tv_sec = static_cast<time_t>(delta.tv_sec + (ms / 1000));
    abstime.tv_nsec = static_cast<long>((delta.tv_usec + (ms % 1000) * 1000) * 1000);

    if (abstime.tv_nsec > 1000000000) {
        abstime.tv_sec += 1;
        abstime.tv_nsec -= 1000000000;
    }

    while (true) {
        ret = pthread_cond_timedwait(&other->cond, &mutex, &abstime);
        if (ret == 0)
            return 0;
        else if (ret == EINTR)
            continue;
        else if (ret == ETIMEDOUT)
            return MUTEX_TIMEDOUT;
        else
            break;
    }

    return -1;
}
