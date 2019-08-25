#include "MacMediaPlayer.h"

///WorkThread
int MacMediaPlayer::messageLoop() {
    while (msgQueue && !msgQueue->isAbortRequest()) {
        Message msg;
        int ret = getMsg(&msg, true);
        ALOGD(MAC_MEDIA_PLAYER_TAG, "%s pop msg what = %s arg1 = %d arg2 = %d obj = %p", __func__, Message::getMsgSimpleName(msg.what), msg.arg1, msg.arg2, msg.obj);
        if (ret == NEGATIVE_EXIT) {
            break;
        }
        if (ret == Message::REQ_QUIT && event) {
            auto *macEvent = dynamic_cast<MacEvent *>(event);
            if (macEvent) {
                macEvent->doExit();
            }
        }
    }
    return POSITIVE;
}

Audio *MacMediaPlayer::createAudio() {
    return new MacAudio();
}

Surface *MacMediaPlayer::createSurface() {
    return new MacSurface();
}

Stream *MacMediaPlayer::createStream() {
    return new MacStream();
}

Options *MacMediaPlayer::createOptions() const {
    return new MacOptions();
}

Event *MacMediaPlayer::createEvent() {
    return new MacEvent();
}

int MacMediaPlayer::eventLoop() {
    if (event) {
        auto *macEvent = dynamic_cast<MacEvent *>(event);
        if (macEvent) {
            return macEvent->eventLoop();
        }
    }
    return NEGATIVE(S_ERROR);
}

