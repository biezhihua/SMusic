#include "MessageQueue.h"

MessageQueue::MessageQueue() {
    ALOGD(__func__);
    mutex = new Mutex();
    queue = new list<Message *>();
}

MessageQueue::~MessageQueue() {
    ALOGD(__func__);
    clearMsgQueue();
    delete queue;
    delete mutex;
}

int MessageQueue::putMsg(Message *msg) {
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
        ALOGE("%s %d %p", __func__, abortRequest, msg);
        return NEGATIVE(S_NULL);
    }
    ALOGD("%s what=%s", __func__, Message::getMsgSimpleName(msg->what));
    if (queue && mutex) {
        queue->push_back(msg);
        mutex->condSignal();
    }
    ALOGD("%s size=%ld", __func__, queue->size());
    return POSITIVE;
}

int MessageQueue::setAbortRequest(bool abortRequest) {
    ALOGD("%s abortRequest=%d", __func__, abortRequest);
    if (mutex) {
        mutex->mutexLock();
        MessageQueue::abortRequest = abortRequest;
        mutex->condSignal();
        mutex->mutexUnLock();
    }
    return POSITIVE;
}

int MessageQueue::clearMsgQueue() {
    ALOGD(__func__);
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
                ret = NEGATIVE(S_CONDITION);
                ALOGE("%s abort=%d", __func__, abortRequest);
                break;
            }
            Message *message = queue->front();
            if (message) {
                queue->pop_front();
                *msg = *message;
                ret = POSITIVE;
                ALOGD("%s success", __func__);
                break;
            } else if (!block) {
                ret = NEGATIVE(S_CONDITION);
                ALOGD("%s not block", __func__);
                break;
            } else {
                ALOGD("%s waiting", __func__);
                mutex->condWait();
            }
        }
        mutex->mutexUnLock();
    }
    return ret;
}


int MessageQueue::removeMsg(int what) {
    ALOGD("%s what=%s", __func__, Message::getMsgSimpleName(what));
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
    ALOGD(__func__);
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
    ALOGD("%s what=%s", __func__, Message::getMsgSimpleName(what));
    auto *message = new Message();
    message->what = what;
    putMsg(message);
    return POSITIVE;
}

int MessageQueue::notifyMsg(int what, int arg1) {
    ALOGD("%s what=%s arg1=%d", __func__, Message::getMsgSimpleName(what), arg1);
    auto *message = new Message();
    message->what = what;
    message->arg1 = arg1;
    putMsg(message);
    return POSITIVE;
}

int MessageQueue::notifyMsg(int what, int arg1, int arg2) {
    ALOGD("%s what=%s arg1=%d arg2=%d", __func__, Message::getMsgSimpleName(what), arg1, arg2);
    auto *message = new Message();
    message->what = what;
    message->arg1 = arg1;
    message->arg2 = arg2;
    putMsg(message);
    return POSITIVE;
}


