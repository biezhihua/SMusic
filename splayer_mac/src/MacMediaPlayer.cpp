#include "MacMediaPlayer.h"

///WorkThread
int MacMediaPlayer::messageLoop() {
    while (true) {
        Message msg;
        int ret = play->getMsg(&msg, true);
        ALOGD(MAC_MEDIA_PLAYER_TAG, "%s pop msg what = %s arg1 = %d arg2 = %d obj = %p", __func__, Message::getMsgSimpleName(msg.what), msg.arg1, msg.arg2, msg.obj);
        if (ret == NEGATIVE_EXIT) {
            break;
        }
    }
    return POSITIVE;
}

AOut *MacMediaPlayer::createAOut() {
    return new MacAudioOut();
}

VOut *MacMediaPlayer::createVOut() {
    return new MacSDL2VideoOut();
}

Pipeline *MacMediaPlayer::createPipeline() {
    return new MacPipeline();
}
