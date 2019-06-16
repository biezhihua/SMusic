
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
        // 设置图像输出表面
        VOut *vOut = createSurface();
        pPlay->setVOut(vOut);
        if (!vOut) {
            return EXIT_FAILURE;
        }

        // 设置数据输入管道
        Pipeline *pipeline = createPipeline();
        if (pipeline) {
            PipelineOpaque *opaque = pipeline->createOpaque();
            if (opaque) {
                opaque->setVOut(vOut);
                pipeline->setOpaque(opaque);
            }
            pPlay->setPipeline(pipeline);
        } else {
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int MediaPlayer::start() {
    ALOGD(__func__);
    if (pPlay && pMutex) {
        pMutex->mutexLock();
        removeMsg(Message::REQ_START);
        removeMsg(Message::REQ_PAUSE);
        notifyMsg1(Message::REQ_START);
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
        notifyMsg1(Message::REQ_PAUSE);
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
    MediaPlayer *mediaPlayer = static_cast<MediaPlayer *>(arg);
    return mediaPlayer->messageLoop();
}

int MediaPlayer::prepareAsync() {
    if (pState && pMutex && pPlay) {
        pMutex->mutexLock();
        pState->changeState(State::STATE_ASYNC_PREPARING);
        if (pPlay->getMsgQueue()) {
            pPlay->getMsgQueue()->start();
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

int MediaPlayer::messageLoop() {
    ALOGD(__func__);
    for (;;) {

    }
    return EXIT_FAILURE;
}

void MediaPlayer::notifyMsg1(int what) {
    if (pPlay && pPlay->getMsgQueue()) {
        MessageQueue *msg = pPlay->getMsgQueue();
        msg->notifyMsg1(what);
    }
}

void MediaPlayer::notifyMsg2(int what, int arg1) {
    if (pPlay && pPlay->getMsgQueue()) {
        MessageQueue *msg = pPlay->getMsgQueue();
        msg->notifyMsg2(what, arg1);
    }
}

void MediaPlayer::notifyMsg3(int what, int arg1, int arg2) {
    if (pPlay && pPlay->getMsgQueue()) {
        MessageQueue *msg = pPlay->getMsgQueue();
        msg->notifyMsg3(what, arg1, arg2);
    }
}

void MediaPlayer::removeMsg(int what) {
    if (pPlay && pPlay->getMsgQueue()) {
        MessageQueue *msg = pPlay->getMsgQueue();
        msg->removeMsg(what);
    }
}


#pragma clang diagnostic pop