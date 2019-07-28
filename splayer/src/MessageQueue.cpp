#include "../include/MessageQueue.h"

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
    int ret = S_FAILURE;
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
        return S_FAILURE;
    }
    ALOGD("%s what=%s", __func__, Message::getMsgSimpleName(msg->what));
    if (queue && mutex) {
        queue->push_back(msg);
        mutex->condSignal();
    }
    ALOGD("%s size=%d", __func__, queue->size());
    return S_SUCCESS;
}

void MessageQueue::setAbortRequest(bool abortRequest) {
    ALOGD("%s abortRequest=%d", __func__, abortRequest);
    if (mutex) {
        mutex->mutexLock();
        MessageQueue::abortRequest = abortRequest;
        mutex->condSignal();
        mutex->mutexUnLock();
    }
}

void MessageQueue::clearMsgQueue() {
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
}

int MessageQueue::getMsg(Message *msg, bool block) {
    int ret = S_FAILURE;
    if (mutex && queue) {
        mutex->mutexLock();
        while (true) {
            if (abortRequest) {
                ret = S_FAILURE;
                ALOGE("%s abort=%d", __func__, abortRequest);
                break;
            }
            Message *message = queue->front();
            if (message) {
                queue->pop_front();
                *msg = *message;
                ret = S_SUCCESS;
                ALOGD("%s success", __func__);
                break;
            } else if (!block) {
                ret = S_FAILURE;
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


void MessageQueue::removeMsg(int what) {
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
    }
}

int MessageQueue::startMsgQueue() {
    ALOGD(__func__);
    if (queue && mutex) {
        mutex->mutexLock();
        abortRequest = false;
        Message *message = new Message();
        message->what = Message::MSG_FLUSH;
        _putMsg(message);
        mutex->mutexUnLock();
        return S_SUCCESS;
    }
    return S_FAILURE;
}

void MessageQueue::notifyMsg(int what) {
    ALOGD("%s what=%s", __func__, Message::getMsgSimpleName(what));
    Message *message = new Message();
    message->what = what;
    putMsg(message);
}

void MessageQueue::notifyMsg(int what, int arg1) {
    ALOGD("%s what=%s arg1=%d", __func__, Message::getMsgSimpleName(what), arg1);
    Message *message = new Message();
    message->what = what;
    message->arg1 = arg1;
    putMsg(message);
}

void MessageQueue::notifyMsg(int what, int arg1, int arg2) {
    ALOGD("%s what=%s arg1=%d arg2=%d", __func__, Message::getMsgSimpleName(what), arg1, arg2);
    Message *message = new Message();
    message->what = what;
    message->arg1 = arg1;
    message->arg2 = arg2;
    putMsg(message);
}


