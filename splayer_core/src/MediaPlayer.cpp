#include "MediaPlayer.h"
#include "Thread.h"

MediaPlayer::MediaPlayer() = default;

MediaPlayer::~MediaPlayer() = default;

int MediaPlayer::create() {
    ALOGD(MEDIA_PLAYER_TAG, __func__);

    state = new State();
    if (!state) {
        ALOGE(MEDIA_PLAYER_TAG, "create state error");
        return NEGATIVE(S_NOT_MEMORY);
    }

    mutex = new Mutex();
    if (!mutex) {
        delete state;
        ALOGE(MEDIA_PLAYER_TAG, "create mutex error");
        return NEGATIVE(S_NOT_MEMORY);
    }

    play = new FFPlay();
    if (!play) {
        delete mutex;
        ALOGE(MEDIA_PLAYER_TAG, "create play error");
        return NEGATIVE(S_NOT_MEMORY);
    }

    // 设置消息
    state->setMsgQueue(play->getMsgQueue());

    Surface *surface = createSurface();
    if (!surface) {
        delete state;
        delete mutex;
        delete play;
        ALOGE(MEDIA_PLAYER_TAG, "create surface error");
        return NEGATIVE(S_NOT_MEMORY);
    }
    play->setSurface(surface);

    Audio *audio = createAudio();
    if (!audio) {
        delete state;
        delete mutex;
        delete surface;
        delete play;
        ALOGE(MEDIA_PLAYER_TAG, "create audio error");
        return NEGATIVE(S_NOT_MEMORY);
    }
    play->setAudio(audio);

    Pipeline *pipeline = createPipeline();
    if (!pipeline) {
        delete state;
        delete mutex;
        delete play;
        delete surface;
        ALOGE(MEDIA_PLAYER_TAG, "create pipeline error");
        return NEGATIVE(S_NOT_MEMORY);
    }
    play->setPipeline(pipeline);
    pipeline->setSurface(surface);

    return POSITIVE;
}

int MediaPlayer::start() {
    ALOGD(MEDIA_PLAYER_TAG, __func__);
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
    ALOGD(MEDIA_PLAYER_TAG, __func__);
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
    ALOGD(MEDIA_PLAYER_TAG, __func__);
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
    ALOGD(MEDIA_PLAYER_TAG, __func__);
    if (play && mutex) {
        if (destroy()) {
            ALOGD(MEDIA_PLAYER_TAG, "reset - destroy success");
            if (create()) {
                ALOGD(MEDIA_PLAYER_TAG, "reset - create success");
                return 0;
            }
            return NEGATIVE(ENOMEM);
        }
        return NEGATIVE(ENOMEM);
    }
    return NEGATIVE(ENOMEM);
}

int MediaPlayer::destroy() {
    ALOGD(MEDIA_PLAYER_TAG, __func__);
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
    ALOGD(MEDIA_PLAYER_TAG, "%s url = %s", "setDataSource", url);
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
        if (!prepareMsgQueue() && !prepareSurface() && !play->preparePipeline(dataSource)) {
            state->changeState(State::STATE_ERROR);
            mutex->mutexUnLock();
            return NEGATIVE(S_CONDITION);
        }
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int MediaPlayer::prepareMsgQueue() {
    MessageQueue *msgQueue = play->getMsgQueue();
    if (!msgQueue) {
        return NEGATIVE(S_NULL);
    }
    int ret = msgQueue->startMsgQueue();
    if (!ret) {
        return ret;
    }
    msgThread = new Thread(staticMsgLoop, this, "Message");
    if (!msgThread) {
        return NEGATIVE(S_NOT_MEMORY);
    }
    return POSITIVE;
}

int MediaPlayer::prepareSurface() {
    if (!play) {
        return NEGATIVE(S_NULL);
    }
    Surface *surface = play->getSurface();
    if (!surface) {
        return NEGATIVE(S_NULL);
    }
    // TODO
    return POSITIVE;
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


