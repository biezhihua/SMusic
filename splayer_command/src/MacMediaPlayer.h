
#ifndef SPLAYER_COMMAND_MYMEDIAPLAYER_H
#define SPLAYER_COMMAND_MYMEDIAPLAYER_H

#include <MediaPlayer.h>

class MyMediaPlayer : public MediaPlayer {

public:
    int messageLoop() override;

    AOut *createAOut() override;

    VOut *createSurface() override;

    Pipeline *createPipeline() override;

};

#endif //SPLAYER_COMMAND_MYMEDIAPLAYER_H
