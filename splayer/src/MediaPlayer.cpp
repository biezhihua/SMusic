#include "MediaPlayer.h"
#include "Thread.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

MediaPlayer::MediaPlayer() {
    ALOGD(__func__);
    state = new State();
    mutex = new Mutex();
    play = new FFPlay();
    if (state && play) {
        state->setMsgQueue(play->getMsgQueue());
    }
}

MediaPlayer::~MediaPlayer() {
    ALOGD(__func__);
    delete play;
    delete mutex;
    delete state;
}

int MediaPlayer::create() {
    ALOGD(__func__);
    if (play) {

        // 创建输出层
        VOut *vOut = createSurface();
        if (!vOut) {
            // TODO
            return AVERROR(ENOMEM);
        }

        // 创建输出层实现
        VOutOpaque *pOutOpaque = vOut->createOpaque();
        vOut->setVOutOpaque(pOutOpaque);

        // 创建数据管道
        Pipeline *pipeline = createPipeline();
        if (!pipeline) {
            // TODO
            return AVERROR(ENOMEM);
        }

        // 创建数据管道实现
        PipelineOpaque *opaque = pipeline->createOpaque();
        if (!opaque) {
            // TODO
            return AVERROR(ENOMEM);
        }

        opaque->setVOut(vOut);
        pipeline->setOpaque(opaque);

        play->setPipeline(pipeline);
        play->setVOut(vOut);

        return 0;
    }
    // TODO
    return AVERROR(ENOMEM);
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
    return AVERROR(ENOMEM);
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
        return AVERROR(ENOMEM);
    }
    return AVERROR(ENOMEM);
}

int MediaPlayer::pause() {
    ALOGD(__func__);
    if (play && mutex) {
        mutex->mutexLock();
        removeMsg(Message::REQ_START);
        removeMsg(Message::REQ_PAUSE);
        notifyMsg(Message::REQ_PAUSE);
        mutex->mutexUnLock();
        return 0;
    }
    return AVERROR(ENOMEM);
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
            return AVERROR(ENOMEM);
        }
        return AVERROR(ENOMEM);
    }
    return AVERROR(ENOMEM);
}

int MediaPlayer::destroy() {
    ALOGD(__func__);
    if (state && mutex && play) {
        mutex->mutexLock();
        // TODO set new surface
        mutex->mutexUnLock();
        play->shutdown();
        return 0;
    }
    return AVERROR(ENOMEM);
}

int MediaPlayer::setDataSource(const char *url) {
    ALOGD("%s url=%s", "setDataSource", url);
    if (state && mutex && play) {
        mutex->mutexLock();
        dataSource = strdup(url);
        if (dataSource) {
            state->changeState(State::STATE_INITIALIZED);
            mutex->mutexUnLock();
            return 0;
        }
        mutex->mutexUnLock();
        return AVERROR(ENOMEM);
    }
    return AVERROR(ENOMEM);
}

static int staticMsgLoop(void *arg) {
    if (arg) {
        MediaPlayer *mediaPlayer = static_cast<MediaPlayer *>(arg);
        return mediaPlayer->messageLoop();
    }
    return S_FAILURE;
}

int MediaPlayer::prepareAsync() {
    if (state && mutex && play) {
        mutex->mutexLock();
        state->changeState(State::STATE_ASYNC_PREPARING);
        MessageQueue *pQueue = play->getMsgQueue();
        if (pQueue) {
            pQueue->startMsgQueue();
        }
        msgThread = new Thread(staticMsgLoop, this, "msg_loop");
        if (play->prepareAsync(dataSource)) {
            mutex->mutexUnLock();
            return S_SUCCESS;
        }
        state->changeState(State::STATE_ERROR);
        mutex->mutexUnLock();
        return S_FAILURE;
    }
    return S_FAILURE;
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