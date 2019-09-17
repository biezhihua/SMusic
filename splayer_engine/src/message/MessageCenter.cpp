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

void MessageCenter::setMsgListener(IMessageListener *msgListener) {
    ALOGD(TAG, "%s listener = %p", __func__, msgListener);
    MessageCenter::msgListener = msgListener;
}

int MessageCenter::start() {
    abortRequest = false;
    if (msgThread == nullptr) {
        msgThread = new Thread(this, Priority_High);
        if (!msgThread) {
            return ERROR_NOT_MEMORY;
        }
        msgThread->start();
    }
    return SUCCESS;
}

int MessageCenter::stop() {
    abortRequest = true;
    if (msgThread != nullptr) {
        // https://baike.baidu.com/item/pthread_join
        msgThread->join();
        delete msgThread;
        msgThread = nullptr;
    }
    return SUCCESS;
}


void MessageCenter::startMsgQueue() {
    ALOGD(TAG, __func__);
    start();
    msgQueue->startMsgQueue();
}

void MessageCenter::stopMsgQueue() {
    ALOGD(TAG, __func__);
    stop();
    msgQueue->clearMsgQueue(nullptr);
}

int MessageCenter::notifyMsg(int what) {
    return msgQueue->notifyMsg(what);
}

int MessageCenter::notifyMsg(int what, int arg1) {
    return msgQueue->notifyMsg(what, arg1);
}

int MessageCenter::notifyMsg(int what, int arg1, int arg2) {
    return msgQueue->notifyMsg(what, arg1, arg2);
}

int MessageCenter::removeMsg(int what) {
    return msgQueue->removeMsg(what);
}

