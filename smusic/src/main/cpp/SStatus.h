//
// Created by biezhihua on 2019/2/11.
//

#ifndef SMUSIC_SPLAYERSTATUS_H
#define SMUSIC_SPLAYERSTATUS_H

#include <pthread.h>
#include "SLog.h"

#define STATE_NONE  (1000)
#define STATE_CREATE (STATE_NONE + 1)
#define STATE_SOURCE (STATE_CREATE + 1)
#define STATE_PRE_START (STATE_SOURCE + 1)
#define STATE_START (STATE_PRE_START + 1)
#define STATE_PRE_PLAY (STATE_START + 1)
#define STATE_PLAY (STATE_PRE_PLAY + 1)
#define STATE_PAUSE (STATE_PLAY + 1)
#define STATE_STOP (STATE_PAUSE + 1)
#define STATE_DESTROY (STATE_STOP + 1)

class SStatus {

private:
    int state = STATE_NONE;

    pthread_mutex_t mutex;

public:

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

    void moveStatusToStop() {
        LOGD("Status: MoveStatusToStop");
        pthread_mutex_lock(&mutex);
        state = STATE_STOP;
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

    bool isCreate() {
        return state == STATE_CREATE;
    }

    bool isSource() {
        return state == STATE_SOURCE;
    }

    bool isStart() {
        return state == STATE_START;
    }

    bool isPlay() {
        return state == STATE_PLAY;
    }

    bool isPause() {
        return state == STATE_PLAY;
    }

    bool isStop() {
        return state == STATE_STOP;
    }

    bool isDestroy() {
        return state == STATE_DESTROY;
    }

    bool isLeastActiveState(int state) {
        if (state != NR_OPEN && state != STATE_CREATE && state != STATE_DESTROY) {
            return this->state >= state;
        }
        return false;
    }
};


#endif //SMUSIC_SPLAYERSTATUS_H
