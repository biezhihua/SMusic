#ifndef SPLAYER_COMMAND_MYMEDIAPLAYER_H
#define SPLAYER_COMMAND_MYMEDIAPLAYER_H

#include "MediaPlayer.h"
#include "MacAudio.h"
#include "MacSurface.h"
#include "MacStream.h"
#include "MacOptions.h"

#define MAC_MEDIA_PLAYER_TAG "MacMPlayer"

class MacMediaPlayer : public MediaPlayer {

public:
    int messageLoop() override;

    Audio *createAudio() override;

    Surface *createSurface() override;

    Stream *createStream() override;

    int prepareAsync() override;

    Options *createOptions() const override;

};

#endif //SPLAYER_COMMAND_MYMEDIAPLAYER_H
