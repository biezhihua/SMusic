#ifndef SPLAYER_IOS_MEDIAPLAYER_H
#define SPLAYER_IOS_MEDIAPLAYER_H

#include "MediaPlayer.h"

class iOSMediaPlayer : MediaPlayer {
    
    const char *const TAG = "[MP][IOS][MediaPlayer]";
private:
    
public:
    class Builder;
};


class iOSMediaPlayer::Builder {
private:
    
    bool debug = true;
    MediaSync *mediaSync = nullptr;
    AudioDevice *audioDevice = nullptr;
    VideoDevice *videoDevice = nullptr;
    IMessageListener *messageListener = nullptr;
    
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
    
    Builder &withVideoDevice(VideoDevice *videoDevice) {
        this->videoDevice = videoDevice;
        return *this;
    }
    
    Builder &withMessageListener(IMessageListener *messageListener) {
        this->messageListener = messageListener;
        return *this;
    }
    
    iOSMediaPlayer *build() {
        ENGINE_DEBUG = debug;
        iOSMediaPlayer *mediaPlayer = new iOSMediaPlayer();
        mediaPlayer->setMediaSync(mediaSync);
        mediaPlayer->setAudioDevice(audioDevice);
        mediaPlayer->setVideoDevice(videoDevice);
        mediaPlayer->setMessageListener(messageListener);
        return mediaPlayer;
    }
};


#endif
