#include "SPlayer.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma ide diagnostic ignored "OCDFAInspection"

void *startDecodeFrameCallback(void *data) {
    LOGD("SPlayer: startDecodeFrameCallback: start");
    SPlayer *sPlayer = (SPlayer *) data;

    if (sPlayer != NULL) {

        SFFmpeg *pFFmpeg = sPlayer->getFFmpeg();
        SStatus *pStatus = sPlayer->getPlayerStatus();
        SOpenSLES *pOpenSLES = sPlayer->getSOpenSLES();
        SJavaMethods *pJavaMethods = sPlayer->getSJavaMethods();

        if (pFFmpeg != NULL && pStatus != NULL && pOpenSLES != NULL && pJavaMethods != NULL) {

            SQueue *pAudioQueue = pFFmpeg->getAudioQueue();

            if (pAudioQueue != NULL) {
                while (pStatus->isLeastActiveState(STATE_PRE_PLAY)) {
                    int result = pFFmpeg->decodeFrame();
                    if (result == S_FUNCTION_BREAK) {
                        while (pStatus->isLeastActiveState(STATE_PLAY)) {
                            if (pAudioQueue->getSize() > 0) {
                                pFFmpeg->sleep();
                                continue;
                            } else {
                                pStatus->moveStatusToPreComplete();
                                break;
                            }
                        }
                    }
                }
            }

            sPlayer->startDecodeThreadComplete = true;

            if (pOpenSLES != NULL &&
                pFFmpeg != NULL &&
                pStatus != NULL &&
                (pStatus->isPreStop() || pStatus->isPreComplete()) &&
                sPlayer->startDecodeMediaInfoThreadComplete &&
                sPlayer->startDecodeThreadComplete &&
                sPlayer->playAudioThreadComplete) {

                pOpenSLES->release();
                pFFmpeg->release();

                if (pStatus->isPreStop()) {
                    /*
                     * 若处于预终止状态，则释放内存，转移到终止状态。
                     */
                    pStatus->moveStatusToStop();
                    if (pJavaMethods != NULL) {
                        pJavaMethods->onCallJavaStop();
                    }
                } else if (pStatus->isPreComplete()) {
                    /*
                    * 若处于预完成状态，则释放内存，转移到状态。
                    */
                    pStatus->moveStatusToComplete();
                    if (pJavaMethods != NULL) {
                        pJavaMethods->onCallJavaComplete();
                    }
                }
            }
        }
    }

    LOGD("SPlayer: startDecodeFrameCallback: end");
    pthread_exit(&sPlayer->startDecodeThread);
    return NULL;
}

void *startDecodeMediaInfoCallback(void *data) {
    LOGD("SPlayer: startDecodeMediaInfoCallback: start");

    SPlayer *sPlayer = (SPlayer *) data;

    if (sPlayer != NULL && sPlayer->getFFmpeg() != NULL) {

        SFFmpeg *pFFmpeg = sPlayer->getFFmpeg();
        SStatus *pStatus = sPlayer->getPlayerStatus();
        SOpenSLES *pOpenSLES = sPlayer->getSOpenSLES();
        SJavaMethods *pJavaMethods = sPlayer->getSJavaMethods();

        if (pFFmpeg != NULL && pStatus != NULL && pOpenSLES != NULL && pJavaMethods != NULL) {

            if (pFFmpeg->decodeMediaInfo() == S_SUCCESS && pStatus->isPreStart()) {
                pStatus->moveStatusToStart();
                sPlayer->getSJavaMethods()->onCallJavaStart();
            } else {
                if (pJavaMethods != NULL) {
                    pJavaMethods->onCallJavaError(ERROR_CODE_DECODE_MEDIA_INFO,
                                                  "Error:SPlayer:DecodeMediaInfoCallback: decodeMediaInfo");
                }
                if (pStatus->isPreStart() || pStatus->isPreStop()) {

                    LOGD("SPlayer: startDecodeMediaInfoCallback: error, prepare to stop");

                    pOpenSLES->release();
                    pFFmpeg->release();
                    pStatus->moveStatusToStop();
                    if (pJavaMethods != NULL) {
                        pJavaMethods->onCallJavaStop();
                    }
                }
            }

            sPlayer->startDecodeMediaInfoThreadComplete = true;

            /**
             * 在加载完媒体信息后，若处于预终止状态，则释放内存，转移到终止状态。
             */
            if (pOpenSLES != NULL &&
                pFFmpeg != NULL &&
                pStatus != NULL &&
                pStatus->isPreStop() &&
                sPlayer->startDecodeMediaInfoThreadComplete) {

                LOGD("SPlayer: startDecodeMediaInfoCallback: prepare to stop");

                pOpenSLES->release();
                pFFmpeg->release();
                pStatus->moveStatusToStop();

                if (pJavaMethods != NULL) {
                    pJavaMethods->onCallJavaStop();
                }
            }
        }
    }

    LOGD("SPlayer: startDecodeMediaInfoCallback: end");
    pthread_exit(&sPlayer->startDecodeMediaInfoThread);
    return NULL;
}


void *seekCallback(void *data) {
    LOGD("SPlayer: seekCallback: start");
    SPlayer *sPlayer = (SPlayer *) data;
    if (sPlayer != NULL && sPlayer->getFFmpeg() != NULL) {
        SFFmpeg *pFFmpeg = sPlayer->getFFmpeg();
        if (pFFmpeg != NULL) {
            pFFmpeg->seek(sPlayer->getSeekMillis());
        }
    }
    LOGD("SPlayer: seekCallback: end");
    pthread_exit(&sPlayer->seekAudioThread);
    return NULL;
}

void *playAudioCallback(void *data) {
    LOGD("SPlayer: playAudioCallback: start");
    SPlayer *sPlayer = (SPlayer *) data;

    if (sPlayer != NULL && sPlayer->getFFmpeg() != NULL) {

        SFFmpeg *pFFmpeg = sPlayer->getFFmpeg();
        SStatus *pStatus = sPlayer->getPlayerStatus();
        SOpenSLES *pOpenSLES = sPlayer->getSOpenSLES();
        SJavaMethods *pJavaMethods = sPlayer->getSJavaMethods();

        if (pStatus != NULL && pFFmpeg != NULL && pOpenSLES != NULL && pJavaMethods != NULL) {

            LOGD("SPlayer: playAudioCallback called: Init OpenSLES");

            SMedia *audio = pFFmpeg->getAudio();
            int sampleRate = 0;
            if (audio != NULL) {
                sampleRate = audio->getSampleRate();
            }

            if (pOpenSLES->init(sampleRate) == S_SUCCESS && pStatus->isPrePlay()) {
                pOpenSLES->play();
                pStatus->moveStatusToPlay();
                if (pJavaMethods != NULL) {
                    pJavaMethods->onCallJavaPlay();
                }
            } else {
                if (pStatus != NULL && pStatus->isPreStop()) {
                    pOpenSLES->release();
                    pFFmpeg->release();
                    pStatus->moveStatusToStop();
                    if (pJavaMethods != NULL) {
                        pJavaMethods->onCallJavaStop();
                    }
                }
            }

            sPlayer->playAudioThreadComplete = true;

            if (pOpenSLES != NULL &&
                pFFmpeg != NULL &&
                pStatus != NULL &&
                pStatus->isPreStop() &&
                sPlayer->startDecodeMediaInfoThreadComplete &&
                sPlayer->startDecodeThreadComplete &&
                sPlayer->playAudioThreadComplete) {

                pOpenSLES->release();
                pFFmpeg->release();

                pStatus->moveStatusToStop();

                if (pJavaMethods != NULL) {
                    pJavaMethods->onCallJavaStop();
                }
            }
        }

        LOGD("SPlayer: playAudioCallback: end");
        pthread_exit(&sPlayer->playAudioThread);
    }
    return NULL;
}

SPlayer::SPlayer(JavaVM *pVm, JNIEnv *env, jobject instance, SJavaMethods *pMethods) {
    pJavaMethods = pMethods;

    create();
}

SPlayer::~SPlayer() {

    destroy();

    pJavaMethods = NULL;
}


void SPlayer::create() {
    LOGD("SPlayer: create: ------------------- ");

    pStatus = new SStatus();
    pFFmpeg = new SFFmpeg(pStatus, pJavaMethods);
    pOpenSLES = new SOpenSLES(pFFmpeg, pStatus, pJavaMethods);

    if (pStatus != NULL) {
        pStatus->moveStatusToPreCreate();
    }
}


void SPlayer::setSource(string *url) {
    LOGD("SPlayer: setSource: ------------------- ");
    if (isValidState() && (pStatus->isCreate() || pStatus->isStop())) {
        if (pFFmpeg != NULL) {
            pFFmpeg->setSource(url);
        }
        pStatus->moveStatusToSource();
    }
}

void SPlayer::start() {
    LOGD("SPlayer: start: ------------------- ");

    if (isValidState() && (pStatus->isSource() || pStatus->isStop() || pStatus->isComplete())) {
        pStatus->moveStatusToPreStart();

        startDecodeMediaInfoThreadComplete = false;
        pthread_create(&startDecodeMediaInfoThread, NULL, startDecodeMediaInfoCallback, this);
    }
}

void SPlayer::play() {
    LOGD("SPlayer: play: ----------------------- ");

    if (isValidState() && pStatus->isStart()) {
        pStatus->moveStatusToPrePlay();

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
    LOGD("SPlayer: pause: ----------------------- ");

    if (isValidState() && pStatus->isPlay()) {
        pOpenSLES->pause();
        pStatus->moveStatusToPause();
        if (pJavaMethods != NULL) {
            pJavaMethods->onCallJavaPause();
        }
    }
}

void SPlayer::stop() {
    LOGD("SPlayer: stop: ----------------------- ");
    if (isValidState() && pStatus->isLeastActiveState(STATE_START)) {
        pStatus->moveStatusToPreStop();
        if (pStatus->isPreStop()) {
            pOpenSLES->stop();
            pFFmpeg->stop();
        }
    }
}

void SPlayer::destroy() {
    LOGD("SPlayer: destroy: ----------------------- ");

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
    LOGD("SPlayer: seek: ----------------------- ");

    if (isValidState() && pStatus->isPlay()) {
        if (pFFmpeg->getTotalTimeMillis() <= 0) {
            LOGD("SPlayer:seek: total time is 0");
            return;
        }
        if (millis >= 0 && millis <= pFFmpeg->getTotalTimeMillis()) {
            seekMillis = millis;
            pthread_create(&seekAudioThread, NULL, seekCallback, this);
        }
        LOGD("SPlayer:seek: %d %f", (int) millis, pFFmpeg->getTotalTimeMillis());
    }
}

int64_t SPlayer::getSeekMillis() const {
    return seekMillis;
}

void SPlayer::volume(int percent) {
    LOGD("SPlayer: volume: ----------------------- ");
    if (percent >= 0 && percent <= 100 && isValidState()) {
        pOpenSLES->volume(percent);
    }
}

void SPlayer::mute(int mute) {
    LOGD("SPlayer: mute: ----------------------- ");
    if (mute >= 0 && isValidState()) {
        pOpenSLES->mute(mute);
    }
}

void SPlayer::speed(double soundSpeed) {
    LOGD("SPlayer: speed: ----------------------- ");
    if (isValidState()) {
        pOpenSLES->setSoundTouchTempo(soundSpeed);
    }
}

bool SPlayer::isValidState() const { return pFFmpeg != NULL && pStatus != NULL && pOpenSLES != NULL; }

void SPlayer::pitch(double soundPitch) {
    LOGD("SPlayer: pitch: ----------------------- ");
    if (isValidState()) {
        pOpenSLES->setSoundTouchPitch(soundPitch);
    }
}

#pragma clang diagnostic pop