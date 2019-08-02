#include "MacMediaPlayer.h"
#include "MacAOut.h"
#include "MacVOut.h"
#include "MacPipeline.h"

///WorkThread
int MyMediaPlayer::messageLoop() {
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

AOut *MyMediaPlayer::createAOut() {
    return new MacAOut();
}

VOut *MyMediaPlayer::createSurface() {
    return new MacVOut();
}

Pipeline *MyMediaPlayer::createPipeline() {
    return new MacPipeline();
}
