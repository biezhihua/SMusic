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
#define STATE_START (STATE_SOURCE + 1)
#define STATE_PLAY (STATE_START + 1)
#define STATE_PAUSE (STATE_PLAY + 1)
#define STATE_STOP (STATE_PAUSE + 1)
#define STATE_DESTROY (STATE_STOP + 1)

class SStatus {

private:
    int state = STATE_NONE;

public:

    void moveStatusToCreate() {
        LOGD("moveStatusToCreate");
        state = STATE_CREATE;
    }

    void moveStatusToSource() {
        LOGD("moveStatusToStart");
        state = STATE_SOURCE;
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
