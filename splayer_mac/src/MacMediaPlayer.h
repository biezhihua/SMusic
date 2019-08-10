
#ifndef SPLAYER_COMMAND_MYMEDIAPLAYER_H
#define SPLAYER_COMMAND_MYMEDIAPLAYER_H

#include <MediaPlayer.h>
#include "MacAudioOut.h"
#include "MacSDL2VideoOut.h"
#include "MacPipeline.h"

#define MAC_MEDIA_PLAYER_TAG "MacMPlayer"

class MacMediaPlayer : public MediaPlayer {

public:
    int messageLoop() override;

    AOut *createAOut() override;

    VOut *createVOut() override;

    Pipeline *createPipeline() override;

};

#endif //SPLAYER_COMMAND_MYMEDIAPLAYER_H
