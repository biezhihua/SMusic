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
            ALOGD(TAG, "[%s] what = %d whatName = %s errorCode = %d", __func__,
                  msg.what,
                  Msg::getMsgSimpleName(msg.what),
                  msg.arg1I);
        }
        switch (msg.what) {
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
            case Msg::MSG_CHANGE_STATUS: {
                mediaPlayer->changeStatus(getStatus(msg.arg1I));
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

PlayerStatus MessageCenter::getStatus(int arg) {
    if (arg == ERRORED) {
        return ERRORED;
    } else if (arg == CREATED) {
        return CREATED;
    } else if (arg == STARTED) {
        return STARTED;
    } else if (arg == PLAYING) {
        return PLAYING;
    } else if (arg == PAUSED) {
        return PAUSED;
    } else if (arg == STOPPED) {
        return STOPPED;
    } else if (arg == DESTROYED) {
        return DESTROYED;
    } else if (arg == IDLED) {
        return IDLED;
    }
    return ERRORED;
}

