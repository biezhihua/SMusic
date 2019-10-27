#include <message/MessageQueue.h>

MessageQueue::MessageQueue() { queue = new list<Msg *>(); }

MessageQueue::~MessageQueue() { delete queue; }

int MessageQueue::putMsg(Msg *msg) {
    int ret;
    mutex.lock();
    ret = _putMsg(msg);
    condition.signal();
    mutex.unlock();
    return ret;
}

int MessageQueue::_putMsg(Msg *msg) {
    if (!msg) {
        return ERROR_PARAMS;
    }
    queue->push_back(msg);
    return SUCCESS;
}

int MessageQueue::clearMsgQueue(IMessageListener *messageListener) {
    mutex.lock();
    if (queue != nullptr) {
        while (!queue->empty()) {
            Msg *message = queue->front();
            if (message != nullptr) {
                queue->pop_front();
                if (messageListener) {
                    messageListener->onMessage(message);
                }
                delete message;
            }
        }
    }
    mutex.unlock();
    return SUCCESS;
}

int MessageQueue::getMsg(Msg *msg, bool block) {
    int ret = SUCCESS;
    mutex.lock();
    while (true) {
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
            ALOGE(TAG, "[%s] queue is null", __func__);
            break;
        }
    }
    mutex.unlock();
    return ret;
}

int MessageQueue::removeMsg(int what) {
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
    return SUCCESS;
}

int MessageQueue::startMsgQueue() {
    mutex.lock();
    auto *message = new Msg();
    message->what = Msg::MSG_FLUSH;
    _putMsg(message);
    mutex.unlock();
    return SUCCESS;
}

int MessageQueue::notifyMsg(int what) {
    auto *message = new Msg();
    message->what = what;
    putMsg(message);
    return SUCCESS;
}

int MessageQueue::notifyMsg(int what, int arg1) {
    auto *message = new Msg();
    message->what = what;
    message->arg1I = arg1;
    putMsg(message);
    return SUCCESS;
}

int MessageQueue::notifyMsg(int what, float arg1) {
    auto *message = new Msg();
    message->what = what;
    message->arg1F = arg1;
    putMsg(message);
    return SUCCESS;
}

int MessageQueue::notifyMsg(int what, int arg1, int arg2) {
    auto *message = new Msg();
    message->what = what;
    message->arg1I = arg1;
    message->arg2I = arg2;
    putMsg(message);
    return SUCCESS;
}
