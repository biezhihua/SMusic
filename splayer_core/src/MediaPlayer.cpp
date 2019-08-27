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
    if (!msgQueue) {
        ALOGE(MEDIA_PLAYER_TAG, "create msg queue error");
        destroy();
        return NEGATIVE(S_NOT_MEMORY);
    }
    state->setMsgQueue(msgQueue);

    if (!options) {

        options = new Options();

        if (!options) {
            ALOGE(MEDIA_PLAYER_TAG, "create options error");
            destroy();
            return NEGATIVE(S_NOT_MEMORY);
        }
    }

    if (!stream) {

        stream = new Stream();

        if (!stream) {
            ALOGE(MEDIA_PLAYER_TAG, "create stream error");
            destroy();
            return NEGATIVE(S_NOT_MEMORY);
        }
    }

    if (surface) {
        surface->setMediaPlayer(this);
        surface->setStream(stream);
        surface->setMsgQueue(msgQueue);
        surface->setOptions(options);
    }

    if (audio) {
        audio->setMediaPlayer(this);
        audio->setStream(stream);
        audio->setMsgQueue(msgQueue);
        audio->setOptions(options);
    }

    if (event) {
        event->setMediaPlayer(this);
        event->setStream(stream);
        event->setSurface(surface);
        event->setMsgQueue(msgQueue);
        event->setOptions(options);
    }

    stream->setAudio(audio);
    stream->setSurface(surface);
    stream->setMsgQueue(msgQueue);
    stream->setOptions(options);

    return POSITIVE;
}

int MediaPlayer::destroy() {
    ALOGD(MEDIA_PLAYER_TAG, __func__);
    if (mutex) {
        mutex->mutexLock();

        stream->shutdown();

        if (event) {
            event->setMediaPlayer(nullptr);
            event->setStream(nullptr);
            event->setSurface(nullptr);
            event->setMsgQueue(nullptr);
            delete event;
            event = nullptr;
        }

        if (audio) {
            audio->destroy();
            audio->setStream(nullptr);
            audio->setMsgQueue(nullptr);
            audio->setOptions(nullptr);
            audio->setMediaPlayer(nullptr);
            stream->setAudio(nullptr);
            delete audio;
            audio = nullptr;
        }

        if (surface) {
            surface->destroy();
            surface->setMediaPlayer(nullptr);
            surface->setStream(nullptr);
            surface->setOptions(nullptr);
            surface->setMsgQueue(nullptr);
            stream->setSurface(nullptr);
            delete surface;
            surface = nullptr;
        }

        stream->destroy();
        stream->setSurface(nullptr);
        stream->setOptions(nullptr);
        stream->setMsgQueue(nullptr);
        delete stream;
        stream = nullptr;

        delete options;
        options = nullptr;

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

        state->setMsgQueue(nullptr);

        if (msgQueue) {
            delete msgQueue;
            msgQueue = nullptr;
        }

        delete state;
        state = nullptr;

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
        removeMsg(Msg::REQ_START);
        removeMsg(Msg::REQ_PAUSE);
        notifyMsg(Msg::REQ_START);
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(ENOMEM);
}

int MediaPlayer::stop() {
    ALOGD(MEDIA_PLAYER_TAG, __func__);
    if (stream && mutex) {
        mutex->mutexLock();
        removeMsg(Msg::REQ_START);
        removeMsg(Msg::REQ_PAUSE);
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
        removeMsg(Msg::REQ_START);
        removeMsg(Msg::REQ_PAUSE);
        notifyMsg(Msg::REQ_PAUSE);
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


int MediaPlayer::prepareAsync() {
    if (state && mutex) {
        mutex->mutexLock();
        state->changeState(State::STATE_ASYNC_PREPARING);
        if (prepareMsgQueue() && prepareOptions() && prepareEvent() && prepareSurface() && prepareAudio() &&
            prepareStream()) {
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
    if (!surface) {
        options->displayDisable = 1;
        options->videoDisable = 1;
    }
    if (!audio) {
        options->audioDisable = 1;
    }
    stream->setOptions(options);
    return notifyMsg(Msg::MSG_OPTIONS_CREATED);
}

int MediaPlayer::prepareMsgQueue() {
    if (!msgQueue->startMsgQueue()) {
        return NEGATIVE(S_NOT_START_MSG_QUEUE);
    }
    if (!(msgThread = new Thread(staticMsgLoop, this, "Msg"))) {
        return NEGATIVE(S_NOT_MEMORY);
    }
    return POSITIVE;
}

int MediaPlayer::prepareEvent() {
    if (event) {
        if (event->create()) {
            notifyMsg(Msg::MSG_EVENT_CREATED);
        } else {
            notifyMsg(Msg::MSG_EVENT_CREATE_FAILURE);
        }
    }
    return POSITIVE;
}

int MediaPlayer::prepareSurface() {
    if (surface) {
        if (surface->create()) {
            notifyMsg(Msg::MSG_SURFACE_CREATED);
        } else {
            notifyMsg(Msg::MSG_SURFACE_CREATE_FAILURE);
        }
    }
    return POSITIVE;
}

int MediaPlayer::prepareAudio() {
    if (audio) {
        if (audio->create()) {
            notifyMsg(Msg::MSG_AUDIO_CREATED);
        } else {
            notifyMsg(Msg::MSG_AUDIO_CREATE_FAILURE);
        }
    }
    return POSITIVE;
}

int MediaPlayer::prepareStream() {
    if (stream) {
        if (stream->create()) {
            notifyMsg(Msg::MSG_STREAM_CREATED);
        } else {
            notifyMsg(Msg::MSG_STREAM_CREATE_FAILURE);
            return NEGATIVE(S_NOT_CREATE_STREAM);
        }
        if (stream->prepareStream(dataSource)) {
            return POSITIVE;
        } else {
            notifyMsg(Msg::MSG_STREAM_FAILURE);
            return NEGATIVE(S_NOT_INIT_VIDEO_STATE);
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

int MediaPlayer::getMsg(Msg *msg, bool block) {
    if (msgQueue) {
        return msgQueue->getMsg(msg, block);
    }
    return NEGATIVE(S_NULL);
}

void MediaPlayer::setMessage(Message *message) {
    MediaPlayer::message = message;
}

void MediaPlayer::setEvent(Event *event) {
    MediaPlayer::event = event;
}

void MediaPlayer::setStream(Stream *stream) {
    MediaPlayer::stream = stream;
}

void MediaPlayer::setAudio(Audio *audio) {
    MediaPlayer::audio = audio;
}

void MediaPlayer::setSurface(Surface *surface) {
    MediaPlayer::surface = surface;
}

void MediaPlayer::setOptions(Options *options) {
    MediaPlayer::options = options;
}

int MediaPlayer::messageLoop() {

    Msg msg;

    for (;;) {

        if (!msgQueue) {
            ALOGD(MEDIA_PLAYER_TAG, "%s message loop break msgQueue = %p", __func__, msgQueue);
            break;
        }

        msg.free();

        int ret = getMsg(&msg, true);

        if (msg.what != -1) {
            ALOGD(MEDIA_PLAYER_TAG, "%s pop msg what = %s arg1 = %d arg2 = %d obj = %p", __func__,
                  Msg::getMsgSimpleName(msg.what), msg.arg1, msg.arg2, msg.obj);
            if (message) {
                message->onMessage(event, &msg);
            }
        }

        if (msg.what == Msg::REQ_QUIT) {
            destroy();
            quit = false;
        }

        if (ret == NEGATIVE(S_ABORT_REQUEST)) {
            ALOGD(MEDIA_PLAYER_TAG, "%s abort request", __func__);
            quit = false;
            break;
        }

        if (msgQueue->isAbortRequest()) {
            ALOGD(MEDIA_PLAYER_TAG, "%s message loop break isAbortRequest", __func__);
            quit = false;
            break;
        }
    }
    return POSITIVE;
}

int MediaPlayer::eventLoop() {
    if (event) {
        return event->eventLoop();
    } else {
        quit = true;
        while (quit) {
            // 模拟主线程实现循环
        }
    }
    return POSITIVE;
}






