#include "MediaPlayer.h"

static int staticMsgLoop(void *arg) {
    if (arg) {
        auto *mediaPlayer = static_cast<MediaPlayer *>(arg);
        return mediaPlayer->messageLoop();
    }
    return NEGATIVE(S_NULL);
}

MediaPlayer::MediaPlayer() = default;

MediaPlayer::~MediaPlayer() = default;

int MediaPlayer::create() {
    ALOGD(MEDIA_PLAYER_TAG, __func__);

    mutex = new Mutex();
    if (!mutex) {
        ALOGE(MEDIA_PLAYER_TAG, "create mutex error");
        destroy();
        return NEGATIVE(S_NOT_MEMORY);
    }

    state = new State();
    if (!state) {
        ALOGE(MEDIA_PLAYER_TAG, "create state error");
        destroy();
        return NEGATIVE(S_NOT_MEMORY);
    }

    msgQueue = new MessageQueue();
    state->setMsgQueue(msgQueue);

    surface = createSurface();
    if (!surface) {
        ALOGE(MEDIA_PLAYER_TAG, "create surface error");
        destroy();
        return NEGATIVE(S_NOT_MEMORY);
    }
    surface->setMediaPlayer(this);

    audio = createAudio();
    if (!audio) {
        ALOGE(MEDIA_PLAYER_TAG, "create audio error");
        destroy();
        return NEGATIVE(S_NOT_MEMORY);
    }

    stream = createStream();
    if (!stream) {
        ALOGE(MEDIA_PLAYER_TAG, "create stream error");
        destroy();
        return NEGATIVE(S_NOT_MEMORY);
    }

    surface->setStream(stream);
    audio->setStream(stream);
    stream->setAudio(audio);
    stream->setSurface(surface);
    stream->setMsgQueue(msgQueue);

    return POSITIVE;
}

int MediaPlayer::destroy() {
    ALOGD(MEDIA_PLAYER_TAG, __func__);
    if (mutex) {
        mutex->mutexLock();

        if (stream && surface && audio) {

            stream->shutdown();

            surface->destroy();
            surface->setMediaPlayer(nullptr);
            surface->setStream(nullptr);
            surface->setOptions(nullptr);
            stream->setSurface(nullptr);
            delete surface;
            surface = nullptr;

            audio->destroy();
            audio->setStream(nullptr);
            stream->setAudio(nullptr);
            delete audio;
            audio = nullptr;

            stream->destroy();
            stream->setSurface(nullptr);
            stream->setOptions(nullptr);
            delete stream;
            stream = nullptr;

            delete options;
            options = nullptr;
        }

        if (dataSource) {
            free(dataSource);
            dataSource = nullptr;
        }

        if (msgQueue) {
            msgQueue->setAbortRequest(true);
        }

        if (msgThread) {
            msgThread->waitThread();
            msgThread = nullptr;
        }

        if (msgQueue) {
            delete msgQueue;
            msgQueue = nullptr;
        }

        if (state) {
            state->setMsgQueue(nullptr);
            delete state;
            state = nullptr;
        }

        mutex->mutexUnLock();
        delete mutex;
        mutex = nullptr;
    }
    ALOGD(MEDIA_PLAYER_TAG, "%s end", __func__);
    return POSITIVE;
}

int MediaPlayer::start() {
    ALOGD(MEDIA_PLAYER_TAG, __func__);
    if (stream && mutex) {
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
    if (stream && mutex) {
        mutex->mutexLock();
        removeMsg(Message::REQ_START);
        removeMsg(Message::REQ_PAUSE);
        if (stream->stop()) {
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
    if (stream && mutex) {
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
    if (stream && mutex) {
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
    if (state && mutex && stream) {
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

Options *MediaPlayer::createOptions() const {
    return new Options();
}

int MediaPlayer::prepareAsync() {
    if (state && mutex && stream) {
        mutex->mutexLock();
        state->changeState(State::STATE_ASYNC_PREPARING);
        if (prepareMsgQueue() && prepareOptions() && prepareSurface() && prepareAudio() && prepareStream()) {
            mutex->mutexUnLock();
            return POSITIVE;
        }
        state->changeState(State::STATE_ERROR);
        mutex->mutexUnLock();
        return NEGATIVE(S_PREPARE_FAILURE);
    }
    return NEGATIVE(S_NULL);
}

int MediaPlayer::prepareOptions() {
    options = createOptions();
    if (options && stream && surface) {
        stream->setOptions(options);
        surface->setOptions(options);
        audio->setOptions(options);
        return notifyMsg(Message::MSG_OPTIONS_CREATED);
    }
    return NEGATIVE(S_NOT_MEMORY);
}

int MediaPlayer::prepareMsgQueue() {
    if (msgQueue) {
        if (!msgQueue->startMsgQueue()) {
            return NEGATIVE(S_NOT_START_MSG_QUEUE);
        }
        if (!(msgThread = new Thread(staticMsgLoop, this, "Message"))) {
            return NEGATIVE(S_NOT_MEMORY);
        }
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int MediaPlayer::prepareSurface() {
    if (surface && surface->create()) {
        return notifyMsg(Message::MSG_SURFACE_CREATED);
    }
    return NEGATIVE(S_NULL);
}

int MediaPlayer::prepareAudio() {
    if (audio && audio->create()) {
        return notifyMsg(Message::MSG_AUDIO_CREATED);
    }
    return NEGATIVE(S_NULL);
}

int MediaPlayer::prepareStream() {
    if (stream && stream->create()) {
        notifyMsg(Message::MSG_STREAM_CREATED);
        if (stream->prepareStream(dataSource)) {
            return POSITIVE;
        }
    }
    return NEGATIVE(S_NULL);
}

int MediaPlayer::notifyMsg(int what) {
    if (msgQueue) {
        return msgQueue->notifyMsg(what);
    }
    return NEGATIVE(S_NULL);
}

int MediaPlayer::notifyMsg(int what, int arg1) {
    if (msgQueue) {
        return msgQueue->notifyMsg(what, arg1);
    }
    return NEGATIVE(S_NULL);
}

int MediaPlayer::notifyMsg(int what, int arg1, int arg2) {
    if (msgQueue) {
        return msgQueue->notifyMsg(what, arg1, arg2);
    }
    return NEGATIVE(S_NULL);
}

int MediaPlayer::removeMsg(int what) {
    if (msgQueue) {
        return msgQueue->removeMsg(what);
    }
    return NEGATIVE(S_NULL);
}

int MediaPlayer::getMsg(Message *msg, bool block) {
    if (msgQueue) {
        return msgQueue->getMsg(msg, block);
    }
    return NEGATIVE_EXIT;
}




