//
// Created by biezhihua on 2019/2/11.
//

#ifndef SMUSIC_SPLAYERSTATUS_H
#define SMUSIC_SPLAYERSTATUS_H

#include <pthread.h>
#include "SLog.h"

#define STATE_NONE  (1000 + 1)
#define STATE_CREATE (1000 + 2)
#define STATE_START (1000 + 2)
#define STATE_PLAY (1000 + 3)
#define STATE_PAUSE (1000 + 4)
#define STATE_STOP (1000 + 5)
#define STATE_DESTROY (1000 + 6)

class SStatus {

private:
    int state = STATE_NONE;

public:

    void moveStatusToCreate() {
        LOGD("moveStatusToCreate");
        state = STATE_CREATE;
    }

    void moveStatusToStart() {
        LOGD("moveStatusToStart");
        state = STATE_START;
    }

    void moveStatusToPlay() {
        LOGD("moveStatusToPlay");
        state = STATE_PLAY;
    }

    void moveStatusToPause() {
        LOGD("moveStatusToPause");
        state = STATE_PAUSE;
    }

    void moveStatusToStop() {
        LOGD("moveStatusToStop");
        state = STATE_STOP;
    }

    void moveStatusToDestroy() {
        LOGD("moveStatusToDestroy");
        state = STATE_DESTROY;
    }

    bool isNone() {
        return state == STATE_NONE;
    }

    bool isCreate() {
        return state == STATE_CREATE;
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

    bool isLeastState(int state) {

        return this->state >= state;
    }
};


#endif //SMUSIC_SPLAYERSTATUS_H
