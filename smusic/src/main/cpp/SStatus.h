//
// Created by biezhihua on 2019/2/11.
//

#ifndef SMUSIC_SPLAYERSTATUS_H
#define SMUSIC_SPLAYERSTATUS_H

#include <pthread.h>
#include "SLog.h"

#define STATE_NONE  (1000)
#define STATE_PRE_CREATE (1001)
#define STATE_CREATE (1002)
#define STATE_SOURCE (1003)
#define STATE_PRE_START (1004)
#define STATE_START (1005)
#define STATE_PRE_PLAY (1006)
#define STATE_PLAY (1007)
#define STATE_PAUSE (1008)
#define STATE_PRE_STOP (1009)
#define STATE_STOP (1010)
#define STATE_PRE_COMPLETE (1011)
#define STATE_COMPLETE (1012)
#define STATE_PRE_DESTROY (1013)
#define STATE_DESTROY (1014)

class SStatus {

private:
    int state = STATE_NONE;

    pthread_mutex_t mutex;

public:

    SStatus() {
        pthread_mutex_init(&mutex, NULL);
    }

    ~SStatus() {
        pthread_mutex_destroy(&mutex);
    }

    void moveStatusToPreCreate() {
        LOGD("Status: MoveStatusToPreCreate");
        pthread_mutex_lock(&mutex);
        state = STATE_PRE_CREATE;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToCreate() {
        LOGD("Status: MoveStatusToCreate");
        pthread_mutex_lock(&mutex);
        state = STATE_CREATE;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToSource() {
        LOGD("Status: MoveStatusToSource");
        pthread_mutex_lock(&mutex);
        state = STATE_SOURCE;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToStart() {
        LOGD("Status: MoveStatusToStart");
        pthread_mutex_lock(&mutex);
        state = STATE_START;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToPreStart() {
        LOGD("Status: MoveStatusToPreStart");
        pthread_mutex_lock(&mutex);
        state = STATE_PRE_START;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToPrePlay() {
        LOGD("Status: MoveStatusToPrePlay");
        pthread_mutex_lock(&mutex);
        state = STATE_PRE_PLAY;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToPlay() {
        LOGD("Status: MoveStatusToPlay");
        pthread_mutex_lock(&mutex);
        state = STATE_PLAY;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToPause() {
        LOGD("Status: MoveStatusToPause");
        pthread_mutex_lock(&mutex);
        state = STATE_PAUSE;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToPreStop() {
        LOGD("Status: MoveStatusToPreStop");
        pthread_mutex_lock(&mutex);
        state = STATE_PRE_STOP;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToStop() {
        LOGD("Status: MoveStatusToStop");
        pthread_mutex_lock(&mutex);
        state = STATE_STOP;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToPreComplete() {
        LOGD("Status: MoveStatusToPreComplete");
        pthread_mutex_lock(&mutex);
        state = STATE_PRE_COMPLETE;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToComplete() {
        LOGD("Status: MoveStatusToComplete");
        pthread_mutex_lock(&mutex);
        state = STATE_COMPLETE;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToPreDestroy() {
        LOGD("Status: MoveStatusToPreDestroy");
        pthread_mutex_lock(&mutex);
        state = STATE_PRE_DESTROY;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToDestroy() {
        LOGD("Status: MoveStatusToDestroy");
        pthread_mutex_lock(&mutex);
        state = STATE_DESTROY;
        pthread_mutex_unlock(&mutex);
    }

    bool isNone() {
        return state == STATE_NONE;
    }

    bool isPreCreate() {
        return state == STATE_PRE_CREATE;
    }

    bool isCreate() {
        return state == STATE_CREATE;
    }

    bool isSource() {
        return state == STATE_SOURCE;
    }

    bool isPreStart() {
        return state == STATE_PRE_START;
    }

    bool isStart() {
        return state == STATE_START;
    }

    bool isPlay() {
        return state == STATE_PLAY;
    }

    bool isPrePlay() {
        return state == STATE_PRE_PLAY;
    }

    bool isPause() {
        return state == STATE_PAUSE;
    }

    bool isPreStop() {
        return state == STATE_PRE_STOP;
    }

    bool isStop() {
        return state == STATE_STOP;
    }

    bool isPreComplete() {
        return state == STATE_PRE_COMPLETE;
    }

    bool isComplete() {
        return state == STATE_COMPLETE;
    }

    bool isPreDestroy() {
        return state == STATE_PRE_DESTROY;
    }

    bool isDestroy() {
        return state == STATE_DESTROY;
    }

    bool isLeastActiveState(int state) {
        if (this->state != STATE_NONE &&
            this->state != STATE_CREATE &&
            this->state != STATE_PRE_STOP &&
            this->state != STATE_STOP &&
            this->state != STATE_PRE_COMPLETE &&
            this->state != STATE_COMPLETE &&
            this->state != STATE_PRE_DESTROY &&
            this->state != STATE_DESTROY
                ) {
            return this->state >= state;
        }
        return false;
    }
};


#endif //SMUSIC_SPLAYERSTATUS_H
