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

int MacMediaPlayer::prepareAsync() {
    int result = MediaPlayer::prepareAsync();
    if (play) {
        auto *surface = dynamic_cast<MacSurface *>(play->getSurface());
        if (surface) {
            return surface->eventLoop();
        }
    }
    return result;
}

Options *MacMediaPlayer::createOptions() const {
    return new MacOptions();
}
