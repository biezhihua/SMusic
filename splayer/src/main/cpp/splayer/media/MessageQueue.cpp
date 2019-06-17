#include "MessageQueue.h"

MessageQueue::MessageQueue() {
    ALOGD(__func__);
    pMutex = new Mutex();
    pQueue = new list<Message *>();
}

MessageQueue::~MessageQueue() {
    ALOGD(__func__);
    clearMsgQueue();
    delete pQueue;
    delete pMutex;
}

int MessageQueue::putMsg(Message *msg) {
    int ret = EXIT_FAILURE;
    if (pMutex && pQueue) {
        pMutex->mutexLock();
        ret = _putMsg(msg);
        pMutex->mutexUnLock();
    }
    return ret;
}

int MessageQueue::_putMsg(Message *msg) {
    if (abortRequest || !msg) {
        ALOGE("%s %d %p", __func__, abortRequest, msg);
        return EXIT_FAILURE;
    }
    ALOGD("%s what=%s", __func__, Message::getMsgSimpleName(msg->what));
    if (pQueue && pMutex) {
        pQueue->push_back(msg);
        pMutex->condSignal();
    }
    ALOGD("%s size=%d", __func__, pQueue->size());
    return EXIT_SUCCESS;
}

void MessageQueue::setAbortRequest(bool abortRequest) {
    ALOGD("%s abortRequest=%d", __func__, abortRequest);
    if (pMutex) {
        pMutex->mutexLock();
        MessageQueue::abortRequest = abortRequest;
        pMutex->condSignal();
        pMutex->mutexUnLock();
    }
}

void MessageQueue::clearMsgQueue() {
    ALOGD(__func__);
    if (pMutex) {
        pMutex->mutexLock();
        if (pQueue != nullptr) {
            while (!pQueue->empty()) {
                Message *message = pQueue->front();
                if (message != nullptr) {
                    pQueue->pop_front();
                    delete message;
                }
            }
        }
        pMutex->mutexUnLock();
    }
}

int MessageQueue::getMsg(Message *msg, bool block) {
    int ret = EXIT_FAILURE;
    if (pMutex && pQueue) {
        pMutex->mutexLock();
        while (true) {
            if (abortRequest) {
                ret = EXIT_FAILURE;
                ALOGE("%s abort=%d", __func__, abortRequest);
                break;
            }
            Message *message = pQueue->front();
            if (message) {
                pQueue->pop_front();
                *msg = *message;
                ret = EXIT_SUCCESS;
                ALOGD("%s success", __func__);
                break;
            } else if (!block) {
                ret = EXIT_FAILURE;
                ALOGD("%s not block", __func__);
                break;
            } else {
                ALOGD("%s waiting", __func__);
                pMutex->condWait();
            }
        }
        pMutex->mutexUnLock();
    }
    return ret;
}


void MessageQueue::removeMsg(int what) {
    ALOGD("%s what=%s", __func__, Message::getMsgSimpleName(what));
    if (pQueue && pMutex) {
        pMutex->mutexLock();
        std::list<Message *>::iterator it;
        Message *message = nullptr;
        for (it = pQueue->begin(); it != pQueue->end(); ++it) {
            if (*it && (*it)->what == what) {
                message = *it;
                break;
            }
        }
        pQueue->remove(message);
        delete message;
        pMutex->mutexUnLock();
    }
}

int MessageQueue::startMsgQueue() {
    ALOGD(__func__);
    if (pQueue && pMutex) {
        pMutex->mutexLock();
        abortRequest = false;
        Message *message = new Message();
        message->what = Message::MSG_FLUSH;
        _putMsg(message);
        pMutex->mutexUnLock();
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

void MessageQueue::notifyMsg1(int what) {
    ALOGD("%s what=%s", __func__, Message::getMsgSimpleName(what));
    Message *message = new Message();
    message->what = what;
    putMsg(message);
}

void MessageQueue::notifyMsg2(int what, int arg1) {
    ALOGD("%s what=%s arg1=%d", __func__, Message::getMsgSimpleName(what), arg1);
    Message *message = new Message();
    message->what = what;
    message->arg1 = arg1;
    putMsg(message);
}

void MessageQueue::notifyMsg3(int what, int arg1, int arg2) {
    ALOGD("%s what=%s arg1=%d arg2=%d", __func__, Message::getMsgSimpleName(what), arg1, arg2);
    Message *message = new Message();
    message->what = what;
    message->arg1 = arg1;
    message->arg2 = arg2;
    putMsg(message);
}


