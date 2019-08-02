#include "MacMediaPlayer.h"
#include "MacAOut.h"
#include "MacVOut.h"
#include "MacPipeline.h"

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
    return new MacAOut();
}

VOut *MacMediaPlayer::createSurface() {
    return new MacVOut();
}

Pipeline *MacMediaPlayer::createPipeline() {
    return new MacPipeline();
}
