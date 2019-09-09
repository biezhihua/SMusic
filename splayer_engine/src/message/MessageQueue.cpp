#include <message/MessageQueue.h>

MessageQueue::MessageQueue() {
    queue = new list<Msg *>();
}

MessageQueue::~MessageQueue() {
    clearMsgQueue();
    delete queue;
}

int MessageQueue::putMsg(Msg *msg) {
    ALOGD(TAG, "%s abort = %d what = %s arg1 = %d arg2 = %d", __func__, abortRequest,
          Msg::getMsgSimpleName(msg->what), msg->arg1, msg->arg2);
    int ret;
    mutex.lock();
    ret = _putMsg(msg);
    mutex.unlock();
    return ret;
}

int MessageQueue::_putMsg(Msg *msg) {
    if (abortRequest || !msg) {
        return -1;
    }
    queue->push_back(msg);
    condition.signal();
    return 0;
}

int MessageQueue::setAbortRequest(bool abortRequest) {
    ALOGD(TAG, "%s abortRequest=%d", __func__, abortRequest);
    mutex.lock();
    MessageQueue::abortRequest = abortRequest;
    condition.signal();
    mutex.unlock();
    return 0;
}

int MessageQueue::clearMsgQueue() {
    ALOGD(TAG, __func__);
    mutex.lock();
    if (queue != nullptr) {
        while (!queue->empty()) {
            Msg *message = queue->front();
            if (message != nullptr) {
                queue->pop_front();
                delete message;
            }
        }
    }
    mutex.unlock();
    return 0;
}

int MessageQueue::getMsg(Msg *msg, bool block) {
    int ret;
    mutex.lock();
    while (true) {
        if (abortRequest) {
            ret = -1;
            break;
        }
        if (queue) {
            Msg *message = queue->front();
            if (message) {
                queue->pop_front();
                *msg = *message;
                ret = 0;
                break;
            } else if (!block) {
                ret = 0;
                break;
            } else {
                condition.wait(mutex);
            }
        } else {
            ALOGE(TAG, "%s queue is null", __func__);
            break;
        }
    }
    mutex.unlock();
    return ret;
}


int MessageQueue::removeMsg(int what) {
    ALOGD(TAG, "%s what=%s", __func__, Msg::getMsgSimpleName(what));
    mutex.lock();
    std::list<Msg *>::iterator it;
    Msg *message = nullptr;
    for (it = queue->begin(); it != queue->end(); ++it) {
        if (*it && (*it)->what == what) {
            message = *it;
            break;
        }
    }
    queue->remove(message);
    delete message;
    mutex.unlock();
    return 0;
}

int MessageQueue::startMsgQueue() {
    ALOGD(TAG, __func__);
    mutex.lock();
    abortRequest = false;
    auto *message = new Msg();
    message->what = Msg::MSG_FLUSH;
    _putMsg(message);
    mutex.unlock();
    return 0;
}

int MessageQueue::notifyMsg(int what) {
    auto *message = new Msg();
    message->what = what;
    putMsg(message);
    return 0;
}

int MessageQueue::notifyMsg(int what, int arg1) {
    auto *message = new Msg();
    message->what = what;
    message->arg1 = arg1;
    putMsg(message);
    return 0;
}

int MessageQueue::notifyMsg(int what, int arg1, int arg2) {
    auto *message = new Msg();
    message->what = what;
    message->arg1 = arg1;
    message->arg2 = arg2;
    putMsg(message);
    return 0;
}

bool MessageQueue::isAbortRequest() const {
    return abortRequest;
}
