#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#ifndef SPLAYER_STATUS_H
#define SPLAYER_STATUS_H

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
#define STATE_SEEK (1008)
#define STATE_PAUSE (1009)
#define STATE_PRE_STOP (1010)
#define STATE_STOP (1011)
#define STATE_PRE_COMPLETE (1012)
#define STATE_COMPLETE (1013)
#define STATE_PRE_DESTROY (1014)
#define STATE_DESTROY (1015)

#include <queue>

#define TAG "Native_Status"

class SStatus {

private:
    std::queue<int> sQueue;

    int prePreState = STATE_NONE;
    int preState = STATE_NONE;
    int state = STATE_NONE;

    pthread_mutex_t mutex;

public:

    SStatus() {
        sQueue.push(STATE_NONE);
        pthread_mutex_init(&mutex, NULL);
    }

    ~SStatus() {
        pthread_mutex_destroy(&mutex);
    }

    void moveStatusToPreCreate() {
        LOGD(TAG, TAG, "Status: MoveStatusToPreCreate");
        pthread_mutex_lock(&mutex);
        if (preState != STATE_NONE) {
            prePreState = preState;
        }
        preState = state;
        state = STATE_PRE_CREATE;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToCreate() {
        LOGD(TAG, "Status: MoveStatusToCreate");
        pthread_mutex_lock(&mutex);
        if (preState != STATE_NONE) {
            prePreState = preState;
        }
        preState = state;
        state = STATE_CREATE;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToSource() {
        LOGD(TAG, "Status: MoveStatusToSource");
        pthread_mutex_lock(&mutex);
        if (preState != STATE_NONE) {
            prePreState = preState;
        }
        preState = state;
        state = STATE_SOURCE;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToStart() {
        LOGD(TAG, "Status: MoveStatusToStart");
        pthread_mutex_lock(&mutex);
        if (preState != STATE_NONE) {
            prePreState = preState;
        }
        preState = state;
        state = STATE_START;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToPreStart() {
        LOGD(TAG, "Status: MoveStatusToPreStart");
        pthread_mutex_lock(&mutex);
        if (preState != STATE_NONE) {
            prePreState = preState;
        }
        preState = state;
        state = STATE_PRE_START;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToPrePlay() {
        LOGD(TAG, "Status: MoveStatusToPrePlay");
        pthread_mutex_lock(&mutex);
        if (preState != STATE_NONE) {
            prePreState = preState;
        }
        preState = state;
        state = STATE_PRE_PLAY;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToPlay() {
        LOGD(TAG, "Status: MoveStatusToPlay");
        pthread_mutex_lock(&mutex);
        if (preState != STATE_NONE) {
            prePreState = preState;
        }
        preState = state;
        state = STATE_PLAY;
        pthread_mutex_unlock(&mutex);
        LOGD(TAG, "Status: MoveStatusToPlay %s %s %s", getState(prePreState), getState(preState), getState(state));
    }

    void moveStatusToSeek() {
        LOGD(TAG, "Status: MoveStatusToSeek");
        pthread_mutex_lock(&mutex);
        if (preState != STATE_NONE) {
            prePreState = preState;
        }
        preState = state;
        state = STATE_SEEK;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToPause() {
        LOGD(TAG, "Status: MoveStatusToPause");
        pthread_mutex_lock(&mutex);
        if (preState != STATE_NONE) {
            prePreState = preState;
        }
        preState = state;
        state = STATE_PAUSE;
        pthread_mutex_unlock(&mutex);
        LOGD(TAG, "Status: MoveStatusToPause %s %s %s", getState(prePreState), getState(preState), getState(state));
    }

    void moveStatusToPreStop() {
        LOGD(TAG, "Status: MoveStatusToPreStop");
        pthread_mutex_lock(&mutex);
        if (preState != STATE_NONE) {
            prePreState = preState;
        }
        preState = state;
        state = STATE_PRE_STOP;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToStop() {
        LOGD(TAG, "Status: MoveStatusToStop");
        pthread_mutex_lock(&mutex);
        if (preState != STATE_NONE) {
            prePreState = preState;
        }
        preState = state;
        state = STATE_STOP;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToPreComplete() {
        LOGD(TAG, "Status: MoveStatusToPreComplete");
        pthread_mutex_lock(&mutex);
        if (preState != STATE_NONE) {
            prePreState = preState;
        }
        preState = state;
        state = STATE_PRE_COMPLETE;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToComplete() {
        LOGD(TAG, "Status: MoveStatusToComplete");
        pthread_mutex_lock(&mutex);
        if (preState != STATE_NONE) {
            prePreState = preState;
        }
        preState = state;
        state = STATE_COMPLETE;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToPreDestroy() {
        LOGD(TAG, "Status: MoveStatusToPreDestroy");
        pthread_mutex_lock(&mutex);
        if (preState != STATE_NONE) {
            prePreState = preState;
        }
        preState = state;
        state = STATE_PRE_DESTROY;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToDestroy() {
        LOGD(TAG, "Status: MoveStatusToDestroy");
        pthread_mutex_lock(&mutex);
        if (preState != STATE_NONE) {
            prePreState = preState;
        }
        preState = state;
        state = STATE_DESTROY;
        pthread_mutex_unlock(&mutex);
    }

    void moveStatusToPreState() {
        pthread_mutex_lock(&mutex);
        int tempPreState = state;
        state = preState;
        preState = tempPreState;
        pthread_mutex_unlock(&mutex);
        LOGD(TAG, "Status: MoveStatusToPreState %s %s %s", getState(prePreState), getState(preState), getState(state));
    }

    const char *getState(int state) {
        switch (state) {
            case STATE_NONE:
                return "STATE_NONE";
            case STATE_PRE_CREATE:
                return "STATE_PRE_CREATE";
            case STATE_CREATE:
                return "STATE_CREATE";
            case STATE_SOURCE:
                return "STATE_SOURCE";
            case STATE_PRE_START:
                return "STATE_PRE_START";
            case STATE_START:
                return "STATE_START";
            case STATE_PRE_PLAY:
                return "STATE_PRE_PLAY";
            case STATE_PLAY:
                return "STATE_PLAY";
            case STATE_SEEK:
                return "STATE_SEEK";
            case STATE_PAUSE:
                return "STATE_PAUSE";
            case STATE_PRE_STOP:
                return "STATE_PRE_STOP";
            case STATE_STOP:
                return "STATE_STOP";
            case STATE_PRE_COMPLETE:
                return "STATE_PRE_COMPLETE";
            case STATE_COMPLETE:
                return "STATE_COMPLETE";
            case STATE_PRE_DESTROY:
                return "STATE_PRE_DESTROY";
            case STATE_DESTROY:
                return "STATE_DESTROY";
            default:
                return "";
        }
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

    bool isSeek() {
        return state == STATE_SEEK;
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

    bool isPrePreSeekState() {
        return prePreState == STATE_SEEK;
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

#endif //SPLAYER_STATUS_H
#pragma clang diagnostic pop