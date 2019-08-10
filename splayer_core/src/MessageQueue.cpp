#include "MessageQueue.h"

MessageQueue::MessageQueue() {
    mutex = new Mutex();
    queue = new list<Message *>();
}

MessageQueue::~MessageQueue() {
    clearMsgQueue();
    delete queue;
    delete mutex;
}

int MessageQueue::putMsg(Message *msg) {

    ALOGD(MESSAGE_QUEUE_TAG, "%s abort = %d what = %s arg1 = %d arg2 = %d", __func__, abortRequest, Message::getMsgSimpleName(msg->what), msg->arg1, msg->arg2);

    int ret = NEGATIVE_UNKNOWN;
    if (mutex && queue) {
        mutex->mutexLock();
        ret = _putMsg(msg);
        mutex->mutexUnLock();
    }
    return ret;
}

int MessageQueue::_putMsg(Message *msg) {
    if (abortRequest || !msg) {
        return NEGATIVE(S_ABORT_REQUEST);
    }
    if (queue && mutex) {
        queue->push_back(msg);
        mutex->condSignal();
    }
    return POSITIVE;
}

int MessageQueue::setAbortRequest(bool abortRequest) {
    ALOGD(MESSAGE_QUEUE_TAG, "%s abortRequest=%d", __func__, abortRequest);
    if (mutex) {
        mutex->mutexLock();
        MessageQueue::abortRequest = abortRequest;
        mutex->condSignal();
        mutex->mutexUnLock();
    }
    return POSITIVE;
}

int MessageQueue::clearMsgQueue() {
    ALOGD(MESSAGE_QUEUE_TAG, __func__);
    if (mutex) {
        mutex->mutexLock();
        if (queue != nullptr) {
            while (!queue->empty()) {
                Message *message = queue->front();
                if (message != nullptr) {
                    queue->pop_front();
                    delete message;
                }
            }
        }
        mutex->mutexUnLock();
    }
    return POSITIVE;
}

int MessageQueue::getMsg(Message *msg, bool block) {
    int ret = NEGATIVE_UNKNOWN;
    if (mutex && queue) {
        mutex->mutexLock();
        while (true) {
            if (abortRequest) {
                ret = NEGATIVE(S_ABORT_REQUEST);
                break;
            }
            Message *message = queue->front();
            if (message) {
                queue->pop_front();
                *msg = *message;
                ret = POSITIVE;
                break;
            } else if (!block) {
                ret = NEGATIVE(S_NOT_BLOACK_GET_MSG);
                break;
            } else {
                mutex->condWait();
            }
        }
        mutex->mutexUnLock();
    }
    if (ret == S_ABORT_REQUEST) {
        ALOGE(MESSAGE_QUEUE_TAG, "%s abort=%d", __func__, abortRequest);
    } else if (ret == POSITIVE) {
        ALOGD(MESSAGE_QUEUE_TAG, "%s success", __func__);
    } else if (ret == S_NOT_BLOACK_GET_MSG) {
        ALOGD(MESSAGE_QUEUE_TAG, "%s not block", __func__);
    } else {
        ALOGD(MESSAGE_QUEUE_TAG, "%s waiting", __func__);
    }
    return ret;
}


int MessageQueue::removeMsg(int what) {
    ALOGD(MESSAGE_QUEUE_TAG, "%s what=%s", __func__, Message::getMsgSimpleName(what));
    if (queue && mutex) {
        mutex->mutexLock();
        std::list<Message *>::iterator it;
        Message *message = nullptr;
        for (it = queue->begin(); it != queue->end(); ++it) {
            if (*it && (*it)->what == what) {
                message = *it;
                break;
            }
        }
        queue->remove(message);
        delete message;
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int MessageQueue::startMsgQueue() {
    ALOGD(MESSAGE_QUEUE_TAG, __func__);
    if (queue && mutex) {
        mutex->mutexLock();
        abortRequest = false;
        auto *message = new Message();
        message->what = Message::MSG_FLUSH;
        _putMsg(message);
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int MessageQueue::notifyMsg(int what) {
    auto *message = new Message();
    message->what = what;
    putMsg(message);
    return POSITIVE;
}

int MessageQueue::notifyMsg(int what, int arg1) {
    auto *message = new Message();
    message->what = what;
    message->arg1 = arg1;
    putMsg(message);
    return POSITIVE;
}

int MessageQueue::notifyMsg(int what, int arg1, int arg2) {
    auto *message = new Message();
    message->what = what;
    message->arg1 = arg1;
    message->arg2 = arg2;
    putMsg(message);
    return POSITIVE;
}


