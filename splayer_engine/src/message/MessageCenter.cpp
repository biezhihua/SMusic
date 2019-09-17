#include <message/MessageCenter.h>

MessageCenter::MessageCenter() {
    msgQueue = new MessageQueue();
}

MessageCenter::~MessageCenter() {
    delete msgQueue;
    msgQueue = nullptr;
}

void MessageCenter::run() {
    ALOGD(TAG, "start message center thread");
    while (!abortRequest) {
        mutex.lock();
        if (!msgQueue) {
            condition.wait(mutex);
        }
        mutex.unlock();
        executeMsg(true);
    }
    ALOGD(TAG, "end message center thread");
}

void MessageCenter::executeMsg(bool block) {
    int ret = msgQueue->getMsg(&msg, block);
    if (ret >= 0 && msg.what != -1) {
        if (msgListener != nullptr) {
            msgListener->onMessage(&msg);
        }
    }
    msg.free();
}

MessageQueue *MessageCenter::getMsgQueue() {
    return msgQueue;
}

void MessageCenter::setMsgListener(IMessageListener *msgListener) {
    ALOGD(TAG, "%s listener = %p", __func__, msgListener);
    MessageCenter::msgListener = msgListener;
}

int MessageCenter::start() {
    mutex.lock();
    abortRequest = false;
    if (!msgThread) {
        msgThread = new Thread(this, Priority_High);
        if (!msgThread) {
            mutex.unlock();
            return ERROR_NOT_MEMORY;
        }
        msgThread->start();
    }
    mutex.unlock();
    return SUCCESS;
}

int MessageCenter::stop() {
    mutex.lock();
    abortRequest = true;
    if (msgThread) {
        // https://baike.baidu.com/item/pthread_join
        msgThread->join();
        delete msgThread;
        msgThread = nullptr;
    }
    mutex.unlock();
    return SUCCESS;
}


void MessageCenter::startMsgQueue() {
    ALOGD(TAG, __func__);
    start();
    if (msgQueue) {
        msgQueue->startMsgQueue();
    }
}

void MessageCenter::stopMsgQueue() {
    ALOGD(TAG, __func__);
    stop();
    if (msgQueue) {
        msgQueue->clearMsgQueue(nullptr);
    }
}

int MessageCenter::notifyMsg(int what) {
    condition.signal();
    if (msgQueue) {
        return msgQueue->notifyMsg(what);
    }
    return ERROR;
}

int MessageCenter::notifyMsg(int what, int arg1) {
    condition.signal();
    if (msgQueue) {
        return msgQueue->notifyMsg(what, arg1);
    }
    return ERROR;
}

int MessageCenter::notifyMsg(int what, int arg1, int arg2) {
    condition.signal();
    if (msgQueue) {
        return msgQueue->notifyMsg(what, arg1, arg2);
    }
    return ERROR;
}

int MessageCenter::removeMsg(int what) {
    condition.signal();
    if (msgQueue) {
        return msgQueue->removeMsg(what);
    }
    return ERROR;
}

