#ifndef SPLAYER_PLAYER_H
#define SPLAYER_PLAYER_H


#include <jni.h>
#include <string>
#include "SJavaMethods.h"
#include "SFFmpeg.h"
#include "SStatus.h"
#include "SError.h"
#include "SOpenSLES.h"
#include <pthread.h>

class SPlayer {

private:

    SJavaMethods *pJavaMethods = NULL;

    SFFmpeg *pFFmpeg = NULL;

    SStatus *pStatus = NULL;

    SOpenSLES *pOpenSLES = NULL;

    int64_t seekMillis = 0;

public:

    bool startDecodeMediaInfoThreadComplete = false;

    bool startDecodeThreadComplete = false;

    bool playAudioThreadComplete = false;

    pthread_t startDecodeMediaInfoThread;

    pthread_t startDecodeThread;

    pthread_t playAudioThread;

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
};


#endif //SPLAYER_PLAYER_H
