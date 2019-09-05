#ifndef SPLAYER_MAC_MACMEDIAPLAYER_H
#define SPLAYER_MAC_MACMEDIAPLAYER_H

#include <player/MediaPlayer.h>
#include "SDLMediaSync.h"

class SDLMediaPlayer : public MediaPlayer {

public:
    class Builder;

    int eventLoop();

};

class SDLMediaPlayer::Builder {
private:

    bool debug = true;
    MediaSync *mediaSync = nullptr;
    AudioDevice *audioDevice = nullptr;

public:

    Builder &withDebug(bool debug) {
        Builder::debug = debug;
        return *this;
    }

    Builder &withMediaSync(MediaSync *sync) {
        this->mediaSync = sync;
        return *this;
    }

    Builder &withAudioDevice(AudioDevice *audioDevice) {
        this->audioDevice = audioDevice;
        return *this;
    }

    SDLMediaPlayer *build() {
        DEBUG = debug;
        SDLMediaPlayer *mediaPlayer = new SDLMediaPlayer();
        mediaPlayer->setMediaSync(mediaSync);
        mediaPlayer->setAudioDevice(audioDevice);
        return mediaPlayer;
    }
};

#endif 
