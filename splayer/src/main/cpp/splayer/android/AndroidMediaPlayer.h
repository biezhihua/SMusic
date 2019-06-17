#ifndef SPLAYER_ANDROIDMEDIAPLAYER_H
#define SPLAYER_ANDROIDMEDIAPLAYER_H

#include "MediaPlayer.h"
#include "AndroidAOut.h"
#include "AndroidVOut.h"
#include "AndroidPipeline.h"
#include <jni.h>

class AndroidMediaPlayer : public MediaPlayer {
private:
    void postEvent(int what, int arg1, int arg2);

    void postEvent2(int what, int arg1, int arg2, jobject obj);

public:
    AndroidMediaPlayer();

    virtual ~AndroidMediaPlayer();

    int messageLoop() override;

protected:
    AOut *createAOut() override;

    VOut *createSurface() override;

    Pipeline *createPipeline() override;
};


#endif //SPLAYER_ANDROIDMEDIAPLAYER_H
