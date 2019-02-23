//
// Created by biezhihua on 2019/2/4.
//

#include "SPlayer.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"

void *startDecodeAudioFrameCallback(void *data) {
    LOGD("SPlayer: startDecodeAudioFrameCallback: start");
    SPlayer *sPlayer = (SPlayer *) data;

    if (sPlayer != NULL) {

        SFFmpeg *pFFmpeg = sPlayer->getSFFmpeg();
        SStatus *pStatus = sPlayer->getPlayerStatus();
        SOpenSLES *pOpenSLES = sPlayer->getSOpenSLES();
        SJavaMethods *pJavaMethods = sPlayer->getSJavaMethods();

        if (pFFmpeg != NULL && pStatus != NULL && pOpenSLES != NULL && pJavaMethods != NULL) {

            SQueue *pAudioQueue = pFFmpeg->getAudioQueue();

            if (pAudioQueue != NULL) {
                while (pStatus->isLeastActiveState(STATE_PRE_PLAY)) {
                    int result = pFFmpeg->decodeAudioFrame();
                    if (result == S_ERROR_BREAK) {
                        while (pStatus->isLeastActiveState(STATE_PLAY)) {
                            if (pAudioQueue->getSize() > 0) {
                                continue;
                            } else {
                                pStatus->moveStatusToPreComplete();
                                break;
                            }
                        }
                    }
                }
            }

            sPlayer->startDecodeAudioThreadComplete = true;

            if (pOpenSLES != NULL &&
                pFFmpeg != NULL &&
                pStatus != NULL &&
                (pStatus->isPreStop() || pStatus->isPreComplete()) &&
                sPlayer->startDecodeMediaInfoThreadComplete &&
                sPlayer->startDecodeAudioThreadComplete &&
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

    LOGD("SPlayer: startDecodeAudioFrameCallback: end");
    pthread_exit(&sPlayer->startDecodeAudioThread);
    return NULL;
}

void *startDecodeMediaInfoCallback(void *data) {
    LOGD("SPlayer: startDecodeMediaInfoCallback: start");

    SPlayer *sPlayer = (SPlayer *) data;

    if (sPlayer != NULL && sPlayer->getSFFmpeg() != NULL) {

        SFFmpeg *pFFmpeg = sPlayer->getSFFmpeg();
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
                if (pStatus->isPreStop()) {
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
    if (sPlayer != NULL && sPlayer->getSFFmpeg() != NULL) {
        SFFmpeg *pFFmpeg = sPlayer->getSFFmpeg();
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

    if (sPlayer != NULL && sPlayer->getSFFmpeg() != NULL) {

        SFFmpeg *pFFmpeg = sPlayer->getSFFmpeg();
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
                sPlayer->startDecodeAudioThreadComplete &&
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
    if (pStatus->isCreate() || pStatus->isStop()) {
        if (pFFmpeg != NULL) {
            pFFmpeg->setSource(url);
        }
        pStatus->moveStatusToSource();
    }
}

void SPlayer::start() {
    LOGD("SPlayer: start: ------------------- ");

    if (pStatus->isSource() || pStatus->isStop() || pStatus->isComplete()) {
        pStatus->moveStatusToPreStart();

        startDecodeMediaInfoThreadComplete = false;
        pthread_create(&startDecodeMediaInfoThread, NULL, startDecodeMediaInfoCallback, this);
    }
}

void SPlayer::play() {
    LOGD("SPlayer: play: ----------------------- ");

    if (pStatus->isStart()) {
        pStatus->moveStatusToPrePlay();

        playAudioThreadComplete = false;
        pthread_create(&playAudioThread, NULL, playAudioCallback, this);

        startDecodeAudioThreadComplete = false;
        pthread_create(&startDecodeAudioThread, NULL, startDecodeAudioFrameCallback, this);

    } else if (pStatus->isPause()) {
        pOpenSLES->play();
        pStatus->moveStatusToPlay();
        if (pJavaMethods != NULL) {
            pJavaMethods->onCallJavaPlay();
        }
    }
}

void SPlayer::pause() {
    LOGD("SPlayer: pause: ----------------------- ");

    if (pStatus->isPlay()) {
        pOpenSLES->pause();
        pStatus->moveStatusToPause();
        if (pJavaMethods != NULL) {
            pJavaMethods->onCallJavaPause();
        }
    }
}

void SPlayer::stop() {
    LOGD("SPlayer: stop: ----------------------- ");
    if (pStatus->isLeastActiveState(STATE_START)) {
        pStatus->moveStatusToPreStop();
        if (pOpenSLES != NULL && pFFmpeg != NULL && pStatus->isPreStop()) {
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

SFFmpeg *SPlayer::getSFFmpeg() {
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

    if (pFFmpeg != NULL && pStatus != NULL && pStatus->isPlay()) {
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
    if (percent >= 0 && percent <= 100 && pFFmpeg != NULL && pStatus != NULL && pOpenSLES != NULL) {
        pOpenSLES->volume(percent);
    }
}

#pragma clang diagnostic pop