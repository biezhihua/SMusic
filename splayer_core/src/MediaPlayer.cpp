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
        return S_ERROR(SE_NOMEM);
    }

    mutex = new Mutex();
    if (!mutex) {
        delete state;
        return S_ERROR(SE_NOMEM);
    }

    play = new FFPlay();
    if (!play) {
        delete mutex;
        return S_ERROR(SE_NOMEM);
    }

    // 设置消息
    state->setMsgQueue(play->getMsgQueue());

    // 创建输出层
    VOut *vOut = createSurface();
    if (!vOut) {
        delete state;
        delete mutex;
        delete play;
        ALOGE("create surface error");
        return S_ERROR(SE_NOMEM);
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
        return S_ERROR(SE_NOMEM);
    }
    play->setPipeline(pipeline);
    pipeline->setVOut(vOut);

    return S_CORRECT;
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
    return S_ERROR(ENOMEM);
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
        return S_ERROR(ENOMEM);
    }
    return S_ERROR(ENOMEM);
}

int MediaPlayer::pause() {
    ALOGD(__func__);
    if (play && mutex) {
        mutex->mutexLock();
        removeMsg(Message::REQ_START);
        removeMsg(Message::REQ_PAUSE);
        notifyMsg(Message::REQ_PAUSE);
        mutex->mutexUnLock();
        return S_CORRECT;
    }
    return S_ERROR(ENOMEM);
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
            return S_ERROR(ENOMEM);
        }
        return S_ERROR(ENOMEM);
    }
    return S_ERROR(ENOMEM);
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
    return S_ERROR(ENOMEM);
}

int MediaPlayer::setDataSource(const char *url) {
    ALOGD("%s url=%s", "setDataSource", url);
    if (state && mutex && play) {
        mutex->mutexLock();
        dataSource = strdup(url);
        if (dataSource) {
            state->changeState(State::STATE_INITIALIZED);
            mutex->mutexUnLock();
            return S_CORRECT;
        }
        mutex->mutexUnLock();
        return S_ERROR(SE_NOMEM);
    }
    return S_ERROR(SE_NULL);
}

static int staticMsgLoop(void *arg) {
    if (arg) {
        auto *mediaPlayer = static_cast<MediaPlayer *>(arg);
        return mediaPlayer->messageLoop();
    }
    return S_ERROR(SE_NULL);
}

int MediaPlayer::prepareAsync() {
    if (state && mutex && play) {
        mutex->mutexLock();
        state->changeState(State::STATE_ASYNC_PREPARING);
        startMsgQueue();
        startMsgQueueThread();
        if (play->prepareAsync(dataSource)) {
            mutex->mutexUnLock();
            return S_CORRECT;
        }
        state->changeState(State::STATE_ERROR);
        mutex->mutexUnLock();
        return S_ERROR(SE_CONDITION);
    }
    return S_ERROR(SE_NULL);
}

int MediaPlayer::startMsgQueueThread() {
    msgThread = new Thread(staticMsgLoop, this, "msg_loop");
    if (msgThread) {
        return S_CORRECT;
    }
    return S_ERROR(SE_NOMEM);
}

int MediaPlayer::startMsgQueue() const {
    MessageQueue *msgQueue = play->getMsgQueue();
    if (msgQueue) {
        return msgQueue->startMsgQueue();
    }
    return S_ERROR(SE_NULL);
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