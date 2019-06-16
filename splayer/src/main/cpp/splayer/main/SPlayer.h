#ifndef SPLAYER_INNERPLAY_H
#define SPLAYER_INNERPLAY_H

#include <jni.h>
#include <string>
#include "SJavaMethods.h"
#include "SFFmpeg.h"
#include "SStatus.h"
#include "SError.h"
#include "SOpenSLES.h"
#include <pthread.h>

#define PLAYER_TAG "Native_Player"

class SPlayer {

private:

    SJavaMethods *pJavaMethods = nullptr;

    SFFmpeg *pFFmpeg = nullptr;

    SStatus *pStatus = nullptr;

    SOpenSLES *pOpenSLES = nullptr;

    int64_t seekMillis = 0;

public:

    bool startDecodeMediaInfoThreadComplete = false;

    bool startDecodeThreadComplete = false;

    bool playAudioThreadComplete = false;

    bool playVideoThreadComplete = false;

    pthread_t startDecodeMediaInfoThread;

    pthread_t startDecodeThread;

    pthread_t playAudioThread;

    pthread_t playVideoThread;

    pthread_t seekAudioThread;

private:
    void create();

    void destroy();

public:
    SPlayer(JavaVM *pVm, JNIEnv *env, jobject instance, SJavaMethods *pMethods);

    ~SPlayer();

    void setSource(string *url);

    void start();

    void play();

    void pause();

    void stop();

    SFFmpeg *getFFmpeg();

    SJavaMethods *getSJavaMethods();

    SStatus *getPlayerStatus();

    SOpenSLES *getSOpenSLES();

    void seek(int64_t millis);

    int64_t getSeekMillis() const;

    void volume(int percent);

    void mute(int mute);

    void speed(double soundSpeed);

    void pitch(double soundPitch);

    bool isValidState() const;

    void tryToStopOrCompleteByStatus();
};


#endif //SPLAYER_INNERPLAY_H
