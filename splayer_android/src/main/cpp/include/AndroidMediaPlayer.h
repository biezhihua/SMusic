#ifndef SPLAYER_ANDROID_MEDIAPLAYER_H
#define SPLAYER_ANDROID_MEDIAPLAYER_H

#include "MediaPlayer.h"
#include <android/native_window.h>
#include "AndroidVideoDevice.h"

class AndroidMediaPlayer : public MediaPlayer {

    const char *const TAG = "[MP][ANDROID][MediaPlayer]";

private:

public:
    class Builder;

    int setVideoSurface(ANativeWindow *nativeWindow);

};

class AndroidMediaPlayer::Builder {
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

    AndroidMediaPlayer *build() {
        DEBUG = debug;
        AndroidMediaPlayer *mediaPlayer = new AndroidMediaPlayer();
        mediaPlayer->setMediaSync(mediaSync);
        mediaPlayer->setAudioDevice(audioDevice);
        mediaPlayer->setVideoDevice(videoDevice);
        mediaPlayer->setMessageListener(messageListener);
        return mediaPlayer;
    }
};

#endif
