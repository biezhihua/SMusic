#include "SPlayer.h"


/**
 * Decode Audio/Video Frame
 */
void *startDecodeFrameCallback(void *data) {

    LOGD(PLAYER_TAG, "SPlayer: startDecodeFrameCallback: start");

    SPlayer *sPlayer = (SPlayer *) data;

    if (sPlayer != nullptr) {

        SFFmpeg *pFFmpeg = sPlayer->getFFmpeg();
        SStatus *pStatus = sPlayer->getPlayerStatus();
        SOpenSLES *pOpenSLES = sPlayer->getSOpenSLES();
        SJavaMethods *pJavaMethods = sPlayer->getSJavaMethods();

        if (pFFmpeg != nullptr && pStatus != nullptr && pOpenSLES != nullptr && pJavaMethods != nullptr) {
            while (pStatus->isLeastActiveState(STATE_PRE_PLAY)) {
                if (pStatus->isPause()) {
                    pFFmpeg->sleep();
                    continue;
                }
                int result = pFFmpeg->decodeFrame();
                if (result == S_FUNCTION_CONTINUE) {
                    // TODO
                } else if (result == S_FUNCTION_BREAK) {
                    LOGD(PLAYER_TAG, "SPlayer: startDecodeFrameCallback-S_FUNCTION_BREAK");
                    while (pStatus->isLeastActiveState(STATE_PLAY)) {
                        SQueue *pAudioQueue = pFFmpeg->getAudioQueue();
                        SQueue *pVideoQueue = pFFmpeg->getVideoQueue();
                        if ((pAudioQueue != nullptr && pAudioQueue->getSize() > 0) ||
                            (pVideoQueue != nullptr && pVideoQueue->getSize() > 0)) {
                            pFFmpeg->sleep();
                            continue;
                        } else {
                            if (pAudioQueue != nullptr) {
                                LOGD(PLAYER_TAG, "audio size = %d", pAudioQueue->getSize());
                            }
                            if (pVideoQueue != nullptr) {
                                LOGD(PLAYER_TAG, "video size = %d", pVideoQueue->getSize());
                            }
                            pStatus->moveStatusToPreComplete();
                            break;
                        }
                    }
                }
            }
            sPlayer->startDecodeThreadComplete = true;
            LOGD(PLAYER_TAG, "SPlayer: startDecodeFrameCallback-tryToStopOrCompleteByStatus");
            sPlayer->tryToStopOrCompleteByStatus();
        }
        LOGD(PLAYER_TAG, "SPlayer: startDecodeFrameCallback: end");
        pthread_exit(&sPlayer->startDecodeThread);
    }
    return nullptr;
}

/**
 * Decode Media Info, Prepare To Play Video/Audio.
 */
void *startDecodeMediaInfoCallback(void *data) {
    LOGD(PLAYER_TAG, "SPlayer: startDecodeMediaInfoCallback: start");

    SPlayer *pPlayer = (SPlayer *) data;

    if (pPlayer != nullptr && pPlayer->getFFmpeg() != nullptr) {

        SFFmpeg *pFFmpeg = pPlayer->getFFmpeg();
        SStatus *pStatus = pPlayer->getPlayerStatus();
        SOpenSLES *pOpenSLES = pPlayer->getSOpenSLES();
        SJavaMethods *pJavaMethods = pPlayer->getSJavaMethods();

        if (pFFmpeg != nullptr && pStatus != nullptr && pOpenSLES != nullptr && pJavaMethods != nullptr) {

            if (pFFmpeg->decodeMediaInfo() == S_SUCCESS) {
                if (pStatus->isPreStart()) {
                    pStatus->moveStatusToStart();
                    pJavaMethods->onCallJavaStart();
                } else if (pStatus->isPreStop()) {
                    // 在加载完媒体信息后，若处于预终止状态，则释放内存，转移到终止状态。
                    LOGD(PLAYER_TAG, "SPlayer: startDecodeMediaInfoCallback: prepare to stop");
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
                    LOGD(PLAYER_TAG, "SPlayer: startDecodeMediaInfoCallback: error, prepare to stop");
                    pOpenSLES->release();
                    pFFmpeg->release();
                    pStatus->moveStatusToStop();
                    pJavaMethods->onCallJavaStop();
                }
                pPlayer->startDecodeMediaInfoThreadComplete = true;
            }
        }
        LOGD(PLAYER_TAG, "SPlayer: startDecodeMediaInfoCallback: end");
        pthread_exit(&pPlayer->startDecodeMediaInfoThread);
    }
    return nullptr;
}

void *seekCallback(void *data) {
    LOGD(PLAYER_TAG, "SPlayer: seekCallback: start");
    SPlayer *sPlayer = (SPlayer *) data;
    if (sPlayer != nullptr && sPlayer->getFFmpeg() != nullptr) {
        SFFmpeg *pFFmpeg = sPlayer->getFFmpeg();
        if (pFFmpeg != nullptr) {
            pFFmpeg->seek(sPlayer->getSeekMillis());
        }
        LOGD(PLAYER_TAG, "SPlayer: seekCallback: end");
        pthread_exit(&sPlayer->seekAudioThread);
    }
    return nullptr;
}

/**
 * Play Audio Frame
 */
void *playAudioCallback(void *data) {
    LOGD(PLAYER_TAG, "SPlayer: playAudioCallback: start");

    SPlayer *pPlayer = (SPlayer *) data;

    if (pPlayer != nullptr && pPlayer->getFFmpeg() != nullptr) {

        SFFmpeg *pFFmpeg = pPlayer->getFFmpeg();
        SStatus *pStatus = pPlayer->getPlayerStatus();
        SOpenSLES *pOpenSLES = pPlayer->getSOpenSLES();
        SJavaMethods *pJavaMethods = pPlayer->getSJavaMethods();

        if (pStatus != nullptr && pFFmpeg != nullptr && pOpenSLES != nullptr && pJavaMethods != nullptr) {
            SMedia *pAudioMedia = pFFmpeg->getAudioMedia();
            if (pStatus->isPrePlay() || pStatus->isPlay()) {
                LOGD(PLAYER_TAG, "SPlayer: playAudioCallback called: Init OpenSLES");
                if (pOpenSLES->init(pAudioMedia != nullptr ? pAudioMedia->getSampleRate() : 0) == S_SUCCESS) {
                    LOGD(PLAYER_TAG, "SPlayer: playAudioCallback : to play");
                    pOpenSLES->play();
                    if (pStatus->isPrePlay()) {
                        pStatus->moveStatusToPlay();
                        pJavaMethods->onCallJavaPlay();
                    }
                }
            }
            pPlayer->playAudioThreadComplete = true;
            LOGD(PLAYER_TAG, "SPlayer: playAudioCallback-tryToStopOrCompleteByStatus");
            pPlayer->tryToStopOrCompleteByStatus();
        }

        LOGD(PLAYER_TAG, "SPlayer: playAudioCallback: end");
        pthread_exit(&pPlayer->playAudioThread);
    }
    return nullptr;
}

/**
 * Play Video Frame
 */
void *playVideoCallback(void *data) {
    LOGD(PLAYER_TAG, "SPlayer: playVideoCallback: start");
    SPlayer *pPlayer = (SPlayer *) data;

    if (pPlayer != nullptr && pPlayer->getFFmpeg() != nullptr) {

        SFFmpeg *pFFmpeg = pPlayer->getFFmpeg();
        SStatus *pStatus = pPlayer->getPlayerStatus();
        SJavaMethods *pJavaMethods = pPlayer->getSJavaMethods();
        SOpenSLES *pOpenSLES = pPlayer->getSOpenSLES();

        if (pStatus != nullptr && pFFmpeg != nullptr && pJavaMethods != nullptr) {

            LOGD(PLAYER_TAG, "SPlayer: playAudioCallback called: Init OpenSLES");

            SMedia *pVideoMedia = pFFmpeg->getVideoMedia();

            if (pVideoMedia != nullptr) {

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
            LOGD(PLAYER_TAG, "SPlayer: playVideoCallback-tryToStopOrCompleteByStatus");
        }

        LOGD(PLAYER_TAG, "SPlayer: playVideoCallback: end");
        pthread_exit(&pPlayer->playVideoThread);
    }
    return nullptr;
}


SPlayer::SPlayer(JavaVM *pVm, JNIEnv *env, jobject instance, SJavaMethods *pMethods) {
    pJavaMethods = pMethods;

    create();
}

SPlayer::~SPlayer() {

    destroy();

    pJavaMethods = nullptr;
}


void SPlayer::create() {
    LOGD(PLAYER_TAG, "SPlayer: create: ------------------- ");

    pStatus = new SStatus();
    pFFmpeg = new SFFmpeg(pStatus, pJavaMethods);
    pOpenSLES = new SOpenSLES(pFFmpeg, pStatus, pJavaMethods);

    if (pStatus != nullptr) {
        pStatus->moveStatusToPreCreate();
    }
}


void SPlayer::setSource(string *url) {
    LOGD(PLAYER_TAG, "SPlayer: setSource: ------------------- ");
    if (isValidState() && (pStatus->isCreate() || pStatus->isStop() || pStatus->isSource())) {
        if (pFFmpeg != nullptr) {
            pFFmpeg->setSource(url);
        }
        pStatus->moveStatusToSource();
    }
}

void SPlayer::start() {
    LOGD(PLAYER_TAG, "SPlayer: start: ------------------- ");

    if (isValidState() && (pStatus->isSource() || pStatus->isStop() || pStatus->isComplete())) {
        pStatus->moveStatusToPreStart();

        startDecodeMediaInfoThreadComplete = false;
        pthread_create(&startDecodeMediaInfoThread, nullptr, startDecodeMediaInfoCallback, this);
    }
}

void SPlayer::play() {
    LOGD(PLAYER_TAG, "SPlayer: play: ----------------------- ");

    if (isValidState() && pStatus->isStart()) {
        pStatus->moveStatusToPrePlay();

        playVideoThreadComplete = false;
        pthread_create(&playVideoThread, nullptr, playVideoCallback, this);

        playAudioThreadComplete = false;
        pthread_create(&playAudioThread, nullptr, playAudioCallback, this);

        startDecodeThreadComplete = false;
        pthread_create(&startDecodeThread, nullptr, startDecodeFrameCallback, this);

    } else if (isValidState() && pStatus->isPause()) {
        pOpenSLES->play();
        pStatus->moveStatusToPlay();
        if (pJavaMethods != nullptr) {
            pJavaMethods->onCallJavaPlay();
        }
    }
}

void SPlayer::pause() {
    LOGD(PLAYER_TAG, "SPlayer: pause: ----------------------- ");

    if (isValidState() && pStatus->isPlay()) {
        pOpenSLES->pause();
        pStatus->moveStatusToPause();
        if (pJavaMethods != nullptr) {
            pJavaMethods->onCallJavaPause();
        }
    }
}

void SPlayer::stop() {
    LOGD(PLAYER_TAG, "SPlayer: stop: ----------------------- ");
    if (isValidState() && pStatus->isLeastActiveState(STATE_START)) {
        pStatus->moveStatusToPreStop();
        if (pStatus->isPreStop()) {
            pOpenSLES->stop();
            pFFmpeg->stop();
        }
    }
}

void SPlayer::destroy() {
    LOGD(PLAYER_TAG, "SPlayer: destroy: ----------------------- ");

    delete pOpenSLES;
    pOpenSLES = nullptr;

    delete pFFmpeg;
    pFFmpeg = nullptr;

    if (pJavaMethods != nullptr) {
        pJavaMethods->onCallJavaDestroy();
    }

    if (pStatus != nullptr) {
        pStatus->moveStatusToDestroy();
    }

    delete pStatus;
    pStatus = nullptr;
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
    LOGD(PLAYER_TAG, "SPlayer: seek: ----------------------- ");

    if (isValidState() && pStatus->isPlay()) {
        if (pFFmpeg->getTotalTimeMillis() <= 0) {
            LOGD(PLAYER_TAG, "SPlayer:seek: total time is 0");
            return;
        }
        if (millis >= 0 && millis <= pFFmpeg->getTotalTimeMillis()) {
            seekMillis = millis;
            pthread_create(&seekAudioThread, nullptr, seekCallback, this);
        }
        LOGD(PLAYER_TAG, "SPlayer:seek: %d %f", (int) millis, pFFmpeg->getTotalTimeMillis());
    }
}

int64_t SPlayer::getSeekMillis() const {
    return seekMillis;
}

void SPlayer::volume(int percent) {
    LOGD(PLAYER_TAG, "SPlayer: volume: ----------------------- ");
    if (percent >= 0 && percent <= 100 && isValidState()) {
        pOpenSLES->volume(percent);
    }
}

void SPlayer::mute(int mute) {
    LOGD(PLAYER_TAG, "SPlayer: mute: ----------------------- ");
    if (mute >= 0 && isValidState()) {
        pOpenSLES->mute(mute);
    }
}

void SPlayer::speed(double soundSpeed) {
    LOGD(PLAYER_TAG, "SPlayer: speed: ----------------------- ");
    if (isValidState()) {
        pOpenSLES->setSoundTouchTempo(soundSpeed);
    }
}

bool SPlayer::isValidState() const { return pFFmpeg != nullptr && pStatus != nullptr && pOpenSLES != nullptr; }

void SPlayer::pitch(double soundPitch) {
    LOGD(PLAYER_TAG, "SPlayer: pitch: ----------------------- ");
    if (isValidState()) {
        pOpenSLES->setSoundTouchPitch(soundPitch);
    }
}

void SPlayer::tryToStopOrCompleteByStatus() {
    LOGD(PLAYER_TAG, "SPlayer: tryToStopOrCompleteByStatus");
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

