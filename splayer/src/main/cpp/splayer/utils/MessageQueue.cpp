#include "MessageQueue.h"

MessageQueue::MessageQueue() {
    pMutex = new Mutex();
    pQueue = new list<Message *>();
    setAbortRequest(true);
}

MessageQueue::~MessageQueue() {
    clear();
    delete pQueue;
    delete pMutex;
}

int MessageQueue::put(Message *msg) {
    int ret = EXIT_FAILURE;
    if (pMutex && pQueue) {
        pMutex->mutexLock();
        ret = _put(msg);
        pMutex->mutexUnLock();
    }
    return ret;
}

int MessageQueue::_put(Message *msg) {
    if (abortRequest || msg) {
        return EXIT_FAILURE;
    }
    if (pQueue && pMutex) {
        pQueue->push_back(msg);
        pMutex->condSignal();
    }
    return EXIT_SUCCESS;
}

void MessageQueue::setAbortRequest(bool abortRequest) {
    if (pMutex) {
        pMutex->mutexLock();
        MessageQueue::abortRequest = abortRequest;
        pMutex->condSignal();
        pMutex->mutexUnLock();
    }
}

void MessageQueue::clear() {
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

int MessageQueue::get(Message *msg, bool block) {
    int ret = EXIT_FAILURE;
    if (pMutex && pQueue) {
        pMutex->mutexLock();
        while (true) {
            if (abortRequest) {
                ret = EXIT_FAILURE;
                break;
            }
            Message *message = pQueue->front();
            if (message) {
                pQueue->pop_front();
                *msg = *message;
                ret = EXIT_SUCCESS;
                break;
            } else if (!block) {
                ret = EXIT_FAILURE;
                break;
            } else {
                pMutex->condWait();
            }
        }
        pMutex->mutexUnLock();
    }
    return ret;
}


void MessageQueue::remove(int what) {
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

void MessageQueue::start() {
    if (pQueue && pMutex) {
        pMutex->mutexLock();
        setAbortRequest(false);
        Message *message = new Message();
        message->what = Message::FFP_MSG_FLUSH;
        _put(message);
        pMutex->mutexUnLock();
    }
}


