#include "SPlayer.h"

#define TAG "Native_Player"

/**
 * Decode Audio/Video Frame
 */
void *startDecodeFrameCallback(void *data) {

    LOGD(TAG, "SPlayer: startDecodeFrameCallback: start");

    SPlayer *sPlayer = (SPlayer *) data;

    if (sPlayer != NULL) {

        SFFmpeg *pFFmpeg = sPlayer->getFFmpeg();
        SStatus *pStatus = sPlayer->getPlayerStatus();
        SOpenSLES *pOpenSLES = sPlayer->getSOpenSLES();
        SJavaMethods *pJavaMethods = sPlayer->getSJavaMethods();

        if (pFFmpeg != NULL && pStatus != NULL && pOpenSLES != NULL && pJavaMethods != NULL) {
            while (pStatus->isLeastActiveState(STATE_PRE_PLAY)) {
                if (pStatus->isPause()) {
                    pFFmpeg->sleep();
                    continue;
                }
                int result = pFFmpeg->decodeFrame();
                if (result == S_FUNCTION_CONTINUE) {
                    // TODO
                } else if (result == S_FUNCTION_BREAK) {
                    LOGD(TAG, "SPlayer: startDecodeFrameCallback-S_FUNCTION_BREAK");
                    while (pStatus->isLeastActiveState(STATE_PLAY)) {
                        SQueue *pAudioQueue = pFFmpeg->getAudioQueue();
                        SQueue *pVideoQueue = pFFmpeg->getVideoQueue();
                        if ((pAudioQueue != NULL && pAudioQueue->getSize() > 0) ||
                            (pVideoQueue != NULL && pVideoQueue->getSize() > 0)) {
                            pFFmpeg->sleep();
                            continue;
                        } else {
                            if (pAudioQueue != NULL) {
                                LOGD(TAG, "audio size = %d", pAudioQueue->getSize());
                            }
                            if (pVideoQueue != NULL) {
                                LOGD(TAG, "video size = %d", pVideoQueue->getSize());
                            }
                            pStatus->moveStatusToPreComplete();
                            break;
                        }
                    }
                }
            }
            sPlayer->startDecodeThreadComplete = true;
            LOGD(TAG, "SPlayer: startDecodeFrameCallback-tryToStopOrCompleteByStatus");
            sPlayer->tryToStopOrCompleteByStatus();
        }
        LOGD(TAG, "SPlayer: startDecodeFrameCallback: end");
        pthread_exit(&sPlayer->startDecodeThread);
    }
    return NULL;
}

/**
 * Decode Media Info, Prepare To Play Video/Audio.
 */
void *startDecodeMediaInfoCallback(void *data) {
    LOGD(TAG, "SPlayer: startDecodeMediaInfoCallback: start");

    SPlayer *pPlayer = (SPlayer *) data;

    if (pPlayer != NULL && pPlayer->getFFmpeg() != NULL) {

        SFFmpeg *pFFmpeg = pPlayer->getFFmpeg();
        SStatus *pStatus = pPlayer->getPlayerStatus();
        SOpenSLES *pOpenSLES = pPlayer->getSOpenSLES();
        SJavaMethods *pJavaMethods = pPlayer->getSJavaMethods();

        if (pFFmpeg != NULL && pStatus != NULL && pOpenSLES != NULL && pJavaMethods != NULL) {

            if (pFFmpeg->decodeMediaInfo() == S_SUCCESS) {
                if (pStatus->isPreStart()) {
                    pStatus->moveStatusToStart();
                    pJavaMethods->onCallJavaStart();
                } else if (pStatus->isPreStop()) {
                    // 在加载完媒体信息后，若处于预终止状态，则释放内存，转移到终止状态。
                    LOGD(TAG, "SPlayer: startDecodeMediaInfoCallback: prepare to stop");
                    pOpenSLES->release();
                    pFFmpeg->release();
                    pStatus->moveStatusToStop();
                    pJavaMethods->onCallJavaStop();
                }
                pPlayer->startDecodeMediaInfoThreadComplete = true;
            } else {
                pJavaMethods->onCallJavaError(ERROR_CODE_DECODE_MEDIA_INFO,
                                              "Error:SPlayer:DecodeMediaInfoCallback: decodeMediaInfo");
                if (pStatus->isPreStart() || pStatus->isPreStop()) {
                    LOGD(TAG, "SPlayer: startDecodeMediaInfoCallback: error, prepare to stop");
                    pOpenSLES->release();
                    pFFmpeg->release();
                    pStatus->moveStatusToStop();
                    pJavaMethods->onCallJavaStop();
                }
                pPlayer->startDecodeMediaInfoThreadComplete = true;
            }
        }
        LOGD(TAG, "SPlayer: startDecodeMediaInfoCallback: end");
        pthread_exit(&pPlayer->startDecodeMediaInfoThread);
    }
    return NULL;
}

void *seekCallback(void *data) {
    LOGD(TAG, "SPlayer: seekCallback: start");
    SPlayer *sPlayer = (SPlayer *) data;
    if (sPlayer != NULL && sPlayer->getFFmpeg() != NULL) {
        SFFmpeg *pFFmpeg = sPlayer->getFFmpeg();
        if (pFFmpeg != NULL) {
            pFFmpeg->seek(sPlayer->getSeekMillis());
        }
        LOGD(TAG, "SPlayer: seekCallback: end");
        pthread_exit(&sPlayer->seekAudioThread);
    }
    return NULL;
}

/**
 * Play Audio Frame
 */
void *playAudioCallback(void *data) {
    LOGD(TAG, "SPlayer: playAudioCallback: start");

    SPlayer *pPlayer = (SPlayer *) data;

    if (pPlayer != NULL && pPlayer->getFFmpeg() != NULL) {

        SFFmpeg *pFFmpeg = pPlayer->getFFmpeg();
        SStatus *pStatus = pPlayer->getPlayerStatus();
        SOpenSLES *pOpenSLES = pPlayer->getSOpenSLES();
        SJavaMethods *pJavaMethods = pPlayer->getSJavaMethods();

        if (pStatus != NULL && pFFmpeg != NULL && pOpenSLES != NULL && pJavaMethods != NULL) {
            SMedia *pAudioMedia = pFFmpeg->getAudioMedia();
            if (pStatus->isPrePlay() || pStatus->isPlay()) {
                LOGD(TAG, "SPlayer: playAudioCallback called: Init OpenSLES");
                if (pOpenSLES->init(pAudioMedia != NULL ? pAudioMedia->getSampleRate() : 0) == S_SUCCESS) {
                    LOGD(TAG, "SPlayer: playAudioCallback : to play");
                    pOpenSLES->play();
                    if (pStatus->isPrePlay()) {
                        pStatus->moveStatusToPlay();
                        pJavaMethods->onCallJavaPlay();
                    }
                }
            }
            pPlayer->playAudioThreadComplete = true;
            LOGD(TAG, "SPlayer: playAudioCallback-tryToStopOrCompleteByStatus");
            pPlayer->tryToStopOrCompleteByStatus();
        }

        LOGD(TAG, "SPlayer: playAudioCallback: end");
        pthread_exit(&pPlayer->playAudioThread);
    }
    return NULL;
}

/**
 * Play Video Frame
 */
void *playVideoCallback(void *data) {
    LOGD(TAG, "SPlayer: playVideoCallback: start");
    SPlayer *pPlayer = (SPlayer *) data;

    if (pPlayer != NULL && pPlayer->getFFmpeg() != NULL) {

        SFFmpeg *pFFmpeg = pPlayer->getFFmpeg();
        SStatus *pStatus = pPlayer->getPlayerStatus();
        SJavaMethods *pJavaMethods = pPlayer->getSJavaMethods();
        SOpenSLES *pOpenSLES = pPlayer->getSOpenSLES();

        if (pStatus != NULL && pFFmpeg != NULL && pJavaMethods != NULL) {

            LOGD(TAG, "SPlayer: playAudioCallback called: Init OpenSLES");

            SMedia *pVideoMedia = pFFmpeg->getVideoMedia();

            if (pVideoMedia != NULL) {

                if (pStatus->isPrePlay()) {
                    pStatus->moveStatusToPlay();
                    pJavaMethods->onCallJavaPlay();
                }

                const char *codecName = pVideoMedia->pCodecContext->codec->name;
                if (pJavaMethods->isSupportMediaCodec(codecName)) {
                    pFFmpeg->startMediaDecode();
                } else {
                    pFFmpeg->startSoftDecode();
                }
            }

            pPlayer->playVideoThreadComplete = true;
            pPlayer->tryToStopOrCompleteByStatus();
            LOGD(TAG, "SPlayer: playVideoCallback-tryToStopOrCompleteByStatus");
        }

        LOGD(TAG, "SPlayer: playVideoCallback: end");
        pthread_exit(&pPlayer->playVideoThread);
    }
    return NULL;
}


SPlayer::SPlayer(JavaVM *pVm, JNIEnv *env, jobject instance, SJavaMethods *pMethods) {
    pJavaMethods = pMethods;

    create();
}

#pragma clang diagnostic pop

SPlayer::~SPlayer() {

    destroy();

    pJavaMethods = NULL;
}


void SPlayer::create() {
    LOGD(TAG, "SPlayer: create: ------------------- ");

    pStatus = new SStatus();
    pFFmpeg = new SFFmpeg(pStatus, pJavaMethods);
    pOpenSLES = new SOpenSLES(pFFmpeg, pStatus, pJavaMethods);

    if (pStatus != NULL) {
        pStatus->moveStatusToPreCreate();
    }
}


void SPlayer::setSource(string *url) {
    LOGD(TAG, "SPlayer: setSource: ------------------- ");
    if (isValidState() && (pStatus->isCreate() || pStatus->isStop() || pStatus->isSource())) {
        if (pFFmpeg != NULL) {
            pFFmpeg->setSource(url);
        }
        pStatus->moveStatusToSource();
    }
}

void SPlayer::start() {
    LOGD(TAG, "SPlayer: start: ------------------- ");

    if (isValidState() && (pStatus->isSource() || pStatus->isStop() || pStatus->isComplete())) {
        pStatus->moveStatusToPreStart();

        startDecodeMediaInfoThreadComplete = false;
        pthread_create(&startDecodeMediaInfoThread, NULL, startDecodeMediaInfoCallback, this);
    }
}

void SPlayer::play() {
    LOGD(TAG, "SPlayer: play: ----------------------- ");

    if (isValidState() && pStatus->isStart()) {
        pStatus->moveStatusToPrePlay();

        playVideoThreadComplete = false;
        pthread_create(&playVideoThread, NULL, playVideoCallback, this);

        playAudioThreadComplete = false;
        pthread_create(&playAudioThread, NULL, playAudioCallback, this);

        startDecodeThreadComplete = false;
        pthread_create(&startDecodeThread, NULL, startDecodeFrameCallback, this);

    } else if (isValidState() && pStatus->isPause()) {
        pOpenSLES->play();
        pStatus->moveStatusToPlay();
        if (pJavaMethods != NULL) {
            pJavaMethods->onCallJavaPlay();
        }
    }
}

void SPlayer::pause() {
    LOGD(TAG, "SPlayer: pause: ----------------------- ");

    if (isValidState() && pStatus->isPlay()) {
        pOpenSLES->pause();
        pStatus->moveStatusToPause();
        if (pJavaMethods != NULL) {
            pJavaMethods->onCallJavaPause();
        }
    }
}

void SPlayer::stop() {
    LOGD(TAG, "SPlayer: stop: ----------------------- ");
    if (isValidState() && pStatus->isLeastActiveState(STATE_START)) {
        pStatus->moveStatusToPreStop();
        if (pStatus->isPreStop()) {
            pOpenSLES->stop();
            pFFmpeg->stop();
        }
    }
}

void SPlayer::destroy() {
    LOGD(TAG, "SPlayer: destroy: ----------------------- ");

    delete pOpenSLES;
    pOpenSLES = NULL;

    delete pFFmpeg;
    pFFmpeg = NULL;

    if (pJavaMethods != NULL) {
        pJavaMethods->onCallJavaDestroy();
    }

    if (pStatus != NULL) {
        pStatus->moveStatusToDestroy();
    }

    delete pStatus;
    pStatus = NULL;
}

SFFmpeg *SPlayer::getFFmpeg() {
    return pFFmpeg;
}

SJavaMethods *SPlayer::getSJavaMethods() {
    return pJavaMethods;
}

SStatus *SPlayer::getPlayerStatus() {
    return pStatus;
}

SOpenSLES *SPlayer::getSOpenSLES() {
    return pOpenSLES;
}

void SPlayer::seek(int64_t millis) {
    LOGD(TAG, "SPlayer: seek: ----------------------- ");

    if (isValidState() && pStatus->isPlay()) {
        if (pFFmpeg->getTotalTimeMillis() <= 0) {
            LOGD(TAG, "SPlayer:seek: total time is 0");
            return;
        }
        if (millis >= 0 && millis <= pFFmpeg->getTotalTimeMillis()) {
            seekMillis = millis;
            pthread_create(&seekAudioThread, NULL, seekCallback, this);
        }
        LOGD(TAG, "SPlayer:seek: %d %f", (int) millis, pFFmpeg->getTotalTimeMillis());
    }
}

int64_t SPlayer::getSeekMillis() const {
    return seekMillis;
}

void SPlayer::volume(int percent) {
    LOGD(TAG, "SPlayer: volume: ----------------------- ");
    if (percent >= 0 && percent <= 100 && isValidState()) {
        pOpenSLES->volume(percent);
    }
}

void SPlayer::mute(int mute) {
    LOGD(TAG, "SPlayer: mute: ----------------------- ");
    if (mute >= 0 && isValidState()) {
        pOpenSLES->mute(mute);
    }
}

void SPlayer::speed(double soundSpeed) {
    LOGD(TAG, "SPlayer: speed: ----------------------- ");
    if (isValidState()) {
        pOpenSLES->setSoundTouchTempo(soundSpeed);
    }
}

bool SPlayer::isValidState() const { return pFFmpeg != NULL && pStatus != NULL && pOpenSLES != NULL; }

void SPlayer::pitch(double soundPitch) {
    LOGD(TAG, "SPlayer: pitch: ----------------------- ");
    if (isValidState()) {
        pOpenSLES->setSoundTouchPitch(soundPitch);
    }
}

void SPlayer::tryToStopOrCompleteByStatus() {
    LOGD(TAG, "SPlayer: tryToStopOrCompleteByStatus");
    if ((pStatus->isPreStop() || pStatus->isPreComplete()) &&
        startDecodeMediaInfoThreadComplete &&
        startDecodeThreadComplete &&
        playVideoThreadComplete &&
        playAudioThreadComplete) {

        pOpenSLES->release();
        pFFmpeg->release();

        if (pStatus->isPreStop()) {
            // 若处于预终止状态，则释放内存，转移到终止状态。
            pStatus->moveStatusToStop();
            pJavaMethods->onCallJavaStop();
        } else if (pStatus->isPreComplete()) {
            // 若处于预完成状态，则释放内存，转移到状态。
            pStatus->moveStatusToComplete();
            pJavaMethods->onCallJavaComplete();
        }
    }
}

