#include "MacMediaPlayer.h"

///WorkThread
int MacMediaPlayer::messageLoop() {
    ALOGD(__func__);
    while (true) {
        Message msg;
        int ret = play->getMsg(&msg, true);
        ALOGD("%s msg=%p", __func__, &msg);
        if (ret == S_ERROR_EXIT) {
            break;
        }
    }
    return S_CORRECT;
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
