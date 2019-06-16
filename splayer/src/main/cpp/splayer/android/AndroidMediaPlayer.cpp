#include "AndroidMediaPlayer.h"

AndroidMediaPlayer::AndroidMediaPlayer() {
    ALOGD(__func__);
}

AndroidMediaPlayer::~AndroidMediaPlayer() {
    ALOGD(__func__);
}

AOut *AndroidMediaPlayer::createAOut() {
    ALOGD(__func__);
    return new AndroidAOut();
}

VOut *AndroidMediaPlayer::createSurface() {
    ALOGD(__func__);
    return new AndroidVOutSurface();
}


Pipeline *AndroidMediaPlayer::createPipeline() {
    ALOGD(__func__);
    return new AndroidPipeline();
}
