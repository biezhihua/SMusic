#ifndef SPLAYER_ANDROIDMEDIAPLAYER_H
#define SPLAYER_ANDROIDMEDIAPLAYER_H

#include "MediaPlayer.h"
#include "AndroidAOut.h"
#include "AndroidVOut.h"
#include "AndroidPipeline.h"

class AndroidMediaPlayer : public MediaPlayer {

public:
    AndroidMediaPlayer();

    virtual ~AndroidMediaPlayer();

protected:
    AOut *createAOut() override;

    VOut *createSurface() override;

    Pipeline *createPipeline() override;
};


#endif //SPLAYER_ANDROIDMEDIAPLAYER_H
