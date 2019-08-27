#ifndef SPLAYER_MAC_MACMEDIAPLAYER_H
#define SPLAYER_MAC_MACMEDIAPLAYER_H

#include "MediaPlayer.h"
#include "MacMessage.h"
#include "MacOptions.h"
#include "MacAudio.h"

class MacMediaPlayer {

public:
    static MediaPlayer *create() {
        MediaPlayer *mediaPlayer = MediaPlayer::Builder{}
                .withMessage(new MacMessage())
                .withOptions(new MacOptions())
                .withEvent(new MacEvent())
                .withAudio(new MacAudio())
                .withSurface(new MacSurface())
                .withDebug(true)
                .build();
        return mediaPlayer;
    }
};

#endif 
