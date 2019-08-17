#include "MediaPlayer.h"

MediaPlayer::MediaPlayer() = default;

MediaPlayer::~MediaPlayer() = default;

int MediaPlayer::create() {
    ALOGD(MEDIA_PLAYER_TAG, __func__);

    state = new State();
    if (!state) {
        ALOGE(MEDIA_PLAYER_TAG, "create state error");
        destroy();
        return NEGATIVE(S_NOT_MEMORY);
    }

    mutex = new Mutex();
    if (!mutex) {
        ALOGE(MEDIA_PLAYER_TAG, "create mutex error");
        destroy();
        return NEGATIVE(S_NOT_MEMORY);
    }

    play = new FFPlay();
    if (!play) {
        ALOGE(MEDIA_PLAYER_TAG, "create play error");
        destroy();
        return NEGATIVE(S_NOT_MEMORY);
    }

    // 设置消息
    state->setMsgQueue(play->getMsgQueue());

    Surface *surface = createSurface();
    if (!surface) {
        ALOGE(MEDIA_PLAYER_TAG, "create surface error");
        destroy();
        return NEGATIVE(S_NOT_MEMORY);
    }
    surface->setPlay(play);
    play->setSurface(surface);

    Audio *audio = createAudio();
    if (!audio) {
        ALOGE(MEDIA_PLAYER_TAG, "create audio error");
        destroy();
        return NEGATIVE(S_NOT_MEMORY);
    }
    audio->setPlay(play);
    play->setAudio(audio);

    Stream *stream = createStream();
    if (!stream) {
        ALOGE(MEDIA_PLAYER_TAG, "create stream error");
        destroy();
        return NEGATIVE(S_NOT_MEMORY);
    }

    stream->setPlay(play);
    stream->setSurface(surface);
    play->setStream(stream);
    return POSITIVE;
}


int MediaPlayer::destroy() {
    ALOGD(MEDIA_PLAYER_TAG, __func__);
    if (mutex && play) {

        play->shutdown();

        if (play->getStream()) {
            Stream *stream = play->getStream();
            stream->destroy();
            stream->setPlay(nullptr);
            stream->setSurface(nullptr);
            play->setStream(nullptr);
            delete stream;
            stream = nullptr;
        }

        if (play->getAudio()) {
            Audio *audio = play->getAudio();
            audio->destroy();
            audio->setPlay(nullptr);
            play->setAudio(nullptr);
            delete audio;
            audio = nullptr;
        }

        if (play->getSurface()) {
            Surface *surface = play->getSurface();
            surface->destroy();
            surface->setPlay(nullptr);
            surface->setOptions(nullptr);
            play->setSurface(nullptr);
            delete surface;
            surface = nullptr;
        }

        if (play->getOptions()) {
            play->setOptions(nullptr);
            delete options;
            options = nullptr;
        }

        delete play;
        play = nullptr;
    }
    if (mutex) {
        delete mutex;
        mutex = nullptr;
    }
    if (state) {
        delete state;
        state = nullptr;
    }
    return NEGATIVE(ENOMEM);
}


int MediaPlayer::start() {
    ALOGD(MEDIA_PLAYER_TAG, __func__);
    if (play && mutex) {
        mutex->mutexLock();
        removeMsg(Message::REQ_START);
        removeMsg(Message::REQ_PAUSE);
        notifyMsg(Message::REQ_START);
        mutex->mutexUnLock();
        return POSITIVE;
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
            return POSITIVE;
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
                return POSITIVE;
            }
            return NEGATIVE(ENOMEM);
        }
        return NEGATIVE(ENOMEM);
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
        if (prepareMsgQueue() && prepareOptions() && prepareSurface() && prepareAudio() && prepareStream()) {
            state->changeState(State::STATE_ERROR);
            mutex->mutexUnLock();
            return NEGATIVE(S_CONDITION);
        }
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int MediaPlayer::prepareOptions() {
    options = createOptions();
    if (options && play) {
        play->setOptions(options);
        if (play->getSurface()) {
            play->getSurface()->setOptions(options);
        }
        return POSITIVE;
    }
    return NEGATIVE(S_NOT_MEMORY);
}

Options *MediaPlayer::createOptions() const {
    return new Options();
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
    if (play && play->getSurface()) {
        Surface *surface = play->getSurface();
        return surface->create();
    }
    return NEGATIVE(S_NULL);
}

int MediaPlayer::prepareStream() {
    if (play && play->getStream()) {
        Stream *stream = play->getStream();
        return stream->create() && play->prepareStream(dataSource);
    }
    return NEGATIVE(S_NULL);
}

int MediaPlayer::prepareAudio() {
    if (play && play->getAudio()) {
        Audio *audio = play->getAudio();
        return audio->create();
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




