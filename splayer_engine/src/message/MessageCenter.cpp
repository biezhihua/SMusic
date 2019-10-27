#include "message/MessageCenter.h"

MessageCenter::MessageCenter(IMediaPlayer *mediaPlayer, ISyncMediaPlayer *innerMediaPlayer) {
    this->msgQueue = new MessageQueue();
    this->mediaPlayer = mediaPlayer;
    this->syncMediaPlayer = innerMediaPlayer;
}

MessageCenter::~MessageCenter() {
    syncMediaPlayer = nullptr;
    mediaPlayer = nullptr;
    delete msgQueue;
    msgQueue = nullptr;
}

void MessageCenter::run() {
    while (!abortRequest) {
        executeMsg(true);
    }
}

void MessageCenter::executeMsg(bool block) {
    int ret = msgQueue->getMsg(&msg, block);
    if (ret >= 0 && msg.what != -1) {
        if (DEBUG) {
            // ALOGD("MediaPlayer_Msg", "[%s] what = %d", __func__, msg.what);
        }
        switch (msg.what) {
            case Msg::MSG_REQUEST_ERROR: {
                int target = msg.arg1I;
                if (Msg::MSG_REQUEST_DESTROY == target) {
                } else if (Msg::MSG_REQUEST_STOP == target) {
                }
                return;
            }
            case Msg::MSG_REQUEST_START: {
                syncMediaPlayer->syncStart();
            }
                break;
            case Msg::MSG_REQUEST_SEEK: {
                syncMediaPlayer->syncSeekTo(msg.arg1F);
            }
                break;
            case Msg::MSG_REQUEST_STOP: {
                syncMediaPlayer->syncStop();
            }
                break;
            case Msg::MSG_REQUEST_PAUSE: {
                syncMediaPlayer->syncPause();
            }
                break;
            case Msg::MSG_REQUEST_PLAY: {
                syncMediaPlayer->syncPlay();
            }
                break;
            case Msg::MSG_STATUS_PREPARE_STOP: {
            }
                break;
            default:
                break;
        }

        if (msgListener != nullptr) {
            msgListener->onMessage(&msg);
        }
    }
    msg.free();
}

void MessageCenter::setMsgListener(IMessageListener *msgListener) {
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
    if (DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    start();
    msgQueue->startMsgQueue();
}

void MessageCenter::stopMsgQueue() {
    if (DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    stop();
    msgQueue->clearMsgQueue(nullptr);
}

int MessageCenter::notifyMsg(int what) { return msgQueue->notifyMsg(what); }

int MessageCenter::notifyMsg(int what, int arg1) {
    return msgQueue->notifyMsg(what, arg1);
}

int MessageCenter::notifyMsg(int what, float arg1) {
    return msgQueue->notifyMsg(what, arg1);
}

int MessageCenter::notifyMsg(int what, int arg1, int arg2) {
    return msgQueue->notifyMsg(what, arg1, arg2);
}

int MessageCenter::removeMsg(int what) { return msgQueue->removeMsg(what); }

