#ifndef SPLAYER_CORE_MEDIAPLAYER_H
#define SPLAYER_CORE_MEDIAPLAYER_H

#include "MediaPlayer.h"
#include "MacAudio.h"
#include "MacSurface.h"
#include "MacStream.h"
#include "MacOptions.h"
#include "MacEvent.h"

#define MAC_MEDIA_PLAYER_TAG "MacMPlayer"

class MacMediaPlayer : public MediaPlayer {

public:
    int messageLoop() override;

    Event *createEvent() override;

    Audio *createAudio() override;

    Surface *createSurface() override;

    Stream *createStream() override;

    int eventLoop();

    Options *createOptions() const override;

};

#endif //SPLAYER_CORE_MEDIAPLAYER_H
