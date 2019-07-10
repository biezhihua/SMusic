#include "State.h"


State::State() {
    ALOGD(__func__);
}

State::~State() {
    ALOGD(__func__);
}

int State::changeState(const int state) {
    ALOGD("%s state=%s", __func__, getState(state));
    State::state = state;
    if (msgQueue) {
        msgQueue->notifyMsg(Message::MSG_PLAYBACK_STATE_CHANGED);
    }
    return EXIT_SUCCESS;
}

const char *State::getState(const int state) {
    switch (state) {
        case State::STATE_INITIALIZED:
            return "STATE_INITIALIZED";
        case State::STATE_ASYNC_PREPARING:
            return "STATE_ASYNC_PREPARING";
        case State::STATE_COMPLETED:
            return "STATE_COMPLETED";
        case State::STATE_END:
            return "STATE_END";
        case State::STATE_ERROR:
            return "STATE_ERROR";
        case State::STATE_IDLE:
            return "STATE_IDLE";
        case State::STATE_PAUSED:
            return "STATE_PAUSED";
        case State::STATE_PREPARED:
            return "STATE_PREPARED";
        case State::STATE_STARTED:
            return "STATE_STARTED";
        case State::STATE_STOPPED:
            return "STATE_STOPPED";
        default:
            return "NONE";
    }
}

void State::setMsgQueue(MessageQueue *msgQueue) {
    State::msgQueue = msgQueue;
}
