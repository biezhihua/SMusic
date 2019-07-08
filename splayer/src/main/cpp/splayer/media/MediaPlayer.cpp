

#include "MediaPlayer.h"
#include "Thread.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

MediaPlayer::MediaPlayer() {
    ALOGD(__func__);
    pState = new State();
    pMutex = new Mutex();
    pPlay = new FFPlay();
    if (pState && pPlay) {
        pState->setMsgQueue(pPlay->getMsgQueue());
    }
}

MediaPlayer::~MediaPlayer() {
    ALOGD(__func__);
    delete pPlay;
    delete pMutex;
    delete pState;
}

int MediaPlayer::create() {
    ALOGD(__func__);
    if (pPlay) {

        // 创建输出层
        VOut *vOut = createSurface();
        if (!vOut) {
            // TODO
            return EXIT_FAILURE;
        }

        // 创建输出层实现
        VOutOpaque *pOutOpaque = vOut->createOpaque();
        vOut->setVOutOpaque(pOutOpaque);

        // 创建数据管道
        Pipeline *pipeline = createPipeline();
        if (!pipeline) {
            // TODO
            return EXIT_FAILURE;
        }

        // 创建数据管道实现
        PipelineOpaque *opaque = pipeline->createOpaque();
        if (!opaque) {
            // TODO
            return EXIT_FAILURE;
        }

        opaque->setVOut(vOut);
        pipeline->setOpaque(opaque);

        pPlay->setPipeline(pipeline);
        pPlay->setVOut(vOut);

        return EXIT_SUCCESS;
    }
    // TODO
    return EXIT_FAILURE;
}

int MediaPlayer::start() {
    ALOGD(__func__);
    if (pPlay && pMutex) {
        pMutex->mutexLock();
        removeMsg(Message::REQ_START);
        removeMsg(Message::REQ_PAUSE);
        notifyMsg(Message::REQ_START);
        pMutex->mutexUnLock();
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int MediaPlayer::stop() {
    ALOGD(__func__);
    if (pPlay && pMutex) {
        pMutex->mutexLock();
        removeMsg(Message::REQ_START);
        removeMsg(Message::REQ_PAUSE);
        if (pPlay->stop()) {
            pState->changeState(State::STATE_STOPPED);
        }
        pMutex->mutexUnLock();
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int MediaPlayer::pause() {
    ALOGD(__func__);
    if (pPlay && pMutex) {
        pMutex->mutexLock();
        removeMsg(Message::REQ_START);
        removeMsg(Message::REQ_PAUSE);
        notifyMsg(Message::REQ_PAUSE);
        pMutex->mutexUnLock();
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int MediaPlayer::reset() {
    ALOGD(__func__);
    if (pPlay && pMutex) {
        if (destroy()) {
            ALOGD("reset - destroy success");
        }
        if (create()) {
            ALOGD("reset - create success");
        }
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int MediaPlayer::destroy() {
    ALOGD(__func__);
    if (pState && pMutex && pPlay) {
        pPlay->shutdown();
        // TODO
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int MediaPlayer::setDataSource(const char *url) {
    ALOGD("%s url=%s", "setDataSource", url);
    if (pState && pMutex && pPlay) {
        pMutex->mutexLock();
        pDataSource = strdup(url);
        if (pDataSource) {
            pState->changeState(State::STATE_INITIALIZED);
            pMutex->mutexUnLock();
            return EXIT_SUCCESS;
        }
        pMutex->mutexUnLock();
        return EXIT_FAILURE;
    }
    return EXIT_FAILURE;
}

static int staticMsgLoop(void *arg) {
    if (arg) {
        MediaPlayer *mediaPlayer = static_cast<MediaPlayer *>(arg);
        return mediaPlayer->messageLoop();
    }
    return EXIT_FAILURE;
}

int MediaPlayer::prepareAsync() {
    if (pState && pMutex && pPlay) {
        pMutex->mutexLock();
        pState->changeState(State::STATE_ASYNC_PREPARING);
        if (pPlay->getMsgQueue()) {
            pPlay->getMsgQueue()->startMsgQueue();
        }
        pMsgThread = new Thread(staticMsgLoop, this, "msg_loop");
        if (pPlay->prepareAsync(pDataSource)) {
            pMutex->mutexUnLock();
            return EXIT_SUCCESS;
        }
        pState->changeState(State::STATE_ERROR);
        pMutex->mutexUnLock();
        return EXIT_FAILURE;
    }
    return EXIT_FAILURE;
}

void MediaPlayer::notifyMsg(int what) {
    if (pPlay && pPlay->getMsgQueue()) {
        MessageQueue *msg = pPlay->getMsgQueue();
        msg->notifyMsg(what);
    }
}

void MediaPlayer::notifyMsg(int what, int arg1) {
    if (pPlay && pPlay->getMsgQueue()) {
        MessageQueue *msg = pPlay->getMsgQueue();
        msg->notifyMsg(what, arg1);
    }
}

void MediaPlayer::notifyMsg(int what, int arg1, int arg2) {
    if (pPlay && pPlay->getMsgQueue()) {
        MessageQueue *msg = pPlay->getMsgQueue();
        msg->notifyMsg(what, arg1, arg2);
    }
}

void MediaPlayer::removeMsg(int what) {
    if (pPlay && pPlay->getMsgQueue()) {
        MessageQueue *msg = pPlay->getMsgQueue();
        msg->removeMsg(what);
    }
}

MessageQueue *MediaPlayer::getMsgQueue() {
    if (pPlay) {
        return pPlay->getMsgQueue();
    }
    return nullptr;
}


#pragma clang diagnostic pop