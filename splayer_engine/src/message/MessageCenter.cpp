#include <message/MessageCenter.h>

MessageCenter::MessageCenter() {
    msgQueue = new MessageQueue();
}

MessageCenter::~MessageCenter() {
    msgQueue->clearMsgQueue();
    delete msgQueue;
    msgQueue = nullptr;
}

void MessageCenter::run() {
    ALOGD(TAG, "%s start message center", __func__);
    Msg msg;
    for (;;) {

        if (!msgQueue) {
            ALOGD(TAG, "%s msgQueue is null, wait set msgQueue", __func__);
            condition.wait(mutex);
            ALOGD(TAG, "%s msgQueue is not null, start message loop", __func__);
        }

        msg.free();

        int ret = msgQueue->getMsg(&msg, true);

        if (ret >= 0 && msg.what != -1) {
            if (msgListener != nullptr) {
                msgListener->onMessage(&msg);
            }
        }

        if (msg.what == Msg::MSG_REQUEST_QUIT) {
            break;
        }
    }
}

MessageQueue *MessageCenter::getMsgQueue() const {
    return msgQueue;
}

void MessageCenter::setMsgQueue(MessageQueue *msgQueue) {
    ALOGD(TAG, __func__);
    MessageCenter::msgQueue = msgQueue;
    condition.signal();
}

void MessageCenter::setMsgListener(IMessageListener *msgListener) {
    ALOGD(TAG, __func__);
    MessageCenter::msgListener = msgListener;
}

int MessageCenter::start() {
    ALOGD(TAG, __func__);
    if (!msgThread) {
        msgThread = new Thread(this);
        if (msgThread) {
            msgThread->start();
            return SUCCESS;
        }
        return ERROR_NOT_MEMORY;
    }
    ALOGD(TAG, "%s message center already started", __func__);
    return SUCCESS;
}

int MessageCenter::stop() {
    if (msgThread) {
        // https://baike.baidu.com/item/pthread_join
        msgThread->join();
        delete msgThread;
        msgThread = nullptr;
    }
    return SUCCESS;
}


void MessageCenter::startMsgQueue() {
    start();
    if (msgQueue) {
        msgQueue->startMsgQueue();
    }
}

void MessageCenter::stopMsgQueue() {
    stop();
    if (msgQueue) {
        msgQueue->clearMsgQueue();
    }
}

