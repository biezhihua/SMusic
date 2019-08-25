#include "MacMediaPlayer.h"

///WorkThread
int MacMediaPlayer::messageLoop() {
    for (;;) {
        if (!msgQueue || msgQueue->isAbortRequest()) {
            ALOGD(MAC_MEDIA_PLAYER_TAG, "%s message loop break msgQueue = %p", __func__, msgQueue);
            break;
        }
        Message msg;
        if (getMsg(&msg, true) == NEGATIVE_EXIT) {
            ALOGD(MAC_MEDIA_PLAYER_TAG, "%s NEGATIVE_EXIT", __func__);
            break;
        }
        ALOGD(MAC_MEDIA_PLAYER_TAG, "%s pop msg what = %s arg1 = %d arg2 = %d obj = %p", __func__, Message::getMsgSimpleName(msg.what), msg.arg1, msg.arg2, msg.obj);
        if (event && msg.what == Message::REQ_QUIT) {
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

