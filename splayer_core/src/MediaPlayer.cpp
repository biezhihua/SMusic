#include "MediaPlayer.h"
#include "Thread.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

MediaPlayer::MediaPlayer() {
    ALOGD(__func__);
}

MediaPlayer::~MediaPlayer() {
    ALOGD(__func__);
}

int MediaPlayer::create() {
    ALOGD(__func__);

    state = new State();
    if (!state) {
        return NEGATIVE(S_NOT_MEMORY);
    }

    mutex = new Mutex();
    if (!mutex) {
        delete state;
        return NEGATIVE(S_NOT_MEMORY);
    }

    play = new FFPlay();
    if (!play) {
        delete mutex;
        return NEGATIVE(S_NOT_MEMORY);
    }

    // 设置消息
    state->setMsgQueue(play->getMsgQueue());

    // 创建输出层
    VOut *vOut = createVOut();
    if (!vOut) {
        delete state;
        delete mutex;
        delete play;
        ALOGE("create surface error");
        return NEGATIVE(S_NOT_MEMORY);
    }
    play->setVOut(vOut);

    // 创建数据管道
    Pipeline *pipeline = createPipeline();
    if (!pipeline) {
        delete state;
        delete mutex;
        delete play;
        delete vOut;
        ALOGE("create pipeline error");
        return NEGATIVE(S_NOT_MEMORY);
    }
    play->setPipeline(pipeline);
    pipeline->setVOut(vOut);

    return POSITIVE;
}

int MediaPlayer::start() {
    ALOGD(__func__);
    if (play && mutex) {
        mutex->mutexLock();
        removeMsg(Message::REQ_START);
        removeMsg(Message::REQ_PAUSE);
        notifyMsg(Message::REQ_START);
        mutex->mutexUnLock();
        return 0;
    }
    return NEGATIVE(ENOMEM);
}

int MediaPlayer::stop() {
    ALOGD(__func__);
    if (play && mutex) {
        mutex->mutexLock();
        removeMsg(Message::REQ_START);
        removeMsg(Message::REQ_PAUSE);
        if (play->stop()) {
            state->changeState(State::STATE_STOPPED);
            mutex->mutexUnLock();
            return 0;
        }
        mutex->mutexUnLock();
        return NEGATIVE(ENOMEM);
    }
    return NEGATIVE(ENOMEM);
}

int MediaPlayer::pause() {
    ALOGD(__func__);
    if (play && mutex) {
        mutex->mutexLock();
        removeMsg(Message::REQ_START);
        removeMsg(Message::REQ_PAUSE);
        notifyMsg(Message::REQ_PAUSE);
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(ENOMEM);
}

int MediaPlayer::reset() {
    ALOGD(__func__);
    if (play && mutex) {
        if (destroy()) {
            ALOGD("reset - destroy success");
            if (create()) {
                ALOGD("reset - create success");
                return 0;
            }
            return NEGATIVE(ENOMEM);
        }
        return NEGATIVE(ENOMEM);
    }
    return NEGATIVE(ENOMEM);
}

int MediaPlayer::destroy() {
    ALOGD(__func__);
    if (state && mutex && play) {
        mutex->mutexLock();
        // TODO set new surface
        mutex->mutexUnLock();
        play->shutdown();

        delete play;
        delete mutex;
        delete state;
        return 0;
    }
    return NEGATIVE(ENOMEM);
}

int MediaPlayer::setDataSource(const char *url) {
    ALOGD("%s url=%s", "setDataSource", url);
    if (state && mutex && play) {
        mutex->mutexLock();
        dataSource = strdup(url);
        if (dataSource) {
            state->changeState(State::STATE_INITIALIZED);
            mutex->mutexUnLock();
            return POSITIVE;
        }
        mutex->mutexUnLock();
        return NEGATIVE(S_NOT_MEMORY);
    }
    return NEGATIVE(S_NULL);
}

static int staticMsgLoop(void *arg) {
    if (arg) {
        auto *mediaPlayer = static_cast<MediaPlayer *>(arg);
        return mediaPlayer->messageLoop();
    }
    return NEGATIVE(S_NULL);
}

int MediaPlayer::prepareAsync() {
    if (state && mutex && play) {
        mutex->mutexLock();
        state->changeState(State::STATE_ASYNC_PREPARING);
        startMsgQueue();
        startMsgQueueThread();
        if (play->prepareAsync(dataSource)) {
            mutex->mutexUnLock();
            return POSITIVE;
        }
        state->changeState(State::STATE_ERROR);
        mutex->mutexUnLock();
        return NEGATIVE(S_CONDITION);
    }
    return NEGATIVE(S_NULL);
}

int MediaPlayer::startMsgQueueThread() {
    msgThread = new Thread(staticMsgLoop, this, "msg_loop");
    if (msgThread) {
        return POSITIVE;
    }
    return NEGATIVE(S_NOT_MEMORY);
}

int MediaPlayer::startMsgQueue() const {
    MessageQueue *msgQueue = play->getMsgQueue();
    if (msgQueue) {
        return msgQueue->startMsgQueue();
    }
    return NEGATIVE(S_NULL);
}

void MediaPlayer::notifyMsg(int what) {
    if (play && play->getMsgQueue()) {
        MessageQueue *msg = play->getMsgQueue();
        msg->notifyMsg(what);
    }
}

void MediaPlayer::notifyMsg(int what, int arg1) {
    if (play && play->getMsgQueue()) {
        MessageQueue *msg = play->getMsgQueue();
        msg->notifyMsg(what, arg1);
    }
}

void MediaPlayer::notifyMsg(int what, int arg1, int arg2) {
    if (play && play->getMsgQueue()) {
        MessageQueue *msg = play->getMsgQueue();
        msg->notifyMsg(what, arg1, arg2);
    }
}

void MediaPlayer::removeMsg(int what) {
    if (play && play->getMsgQueue()) {
        MessageQueue *msg = play->getMsgQueue();
        msg->removeMsg(what);
    }
}

MessageQueue *MediaPlayer::getMsgQueue() {
    if (play) {
        return play->getMsgQueue();
    }
    return nullptr;
}

#pragma clang diagnostic pop