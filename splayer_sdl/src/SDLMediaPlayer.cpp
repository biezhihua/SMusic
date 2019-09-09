#include <SDLMediaPlayer.h>

int SDLMediaPlayer::eventLoop() {
    if (getMediaSync() != nullptr) {
        getMediaSync()->run();
    }
    return 0;
}

void SDLMediaPlayer::onMessage(Msg *msg) {
    ALOGD(TAG, "%s what = %s arg1 = %d arg2 = %d", __func__,
          Msg::getMsgSimpleName(msg->what), msg->arg1, msg->arg2);

    if (msg->what == Msg::MSG_REQUEST_PAUSE) {
        if (isPlaying()) {
            pause();
        } else {
            resume();
        }
    }
}
