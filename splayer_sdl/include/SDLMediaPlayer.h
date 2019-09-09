#ifndef SPLAYER_MAC_MACMEDIAPLAYER_H
#define SPLAYER_MAC_MACMEDIAPLAYER_H

#include <player/MediaPlayer.h>
#include <message/MessageDevice.h>
#include <message/IMessageListener.h>

class SDLMediaPlayer : public MediaPlayer, IMessageListener {

    const char *const TAG = "SDLMediaPlayer";

public:
    class Builder;

    int eventLoop();

private:
    void onMessage(Msg *msg) override;

};

class SDLMediaPlayer::Builder {
private:

    bool debug = true;
    MediaSync *mediaSync = nullptr;
    AudioDevice *audioDevice = nullptr;
    VideoDevice *videoDevice = nullptr;
    MessageDevice *messageDevice = nullptr;

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

    Builder &withMessageDevice(MessageDevice *messageDevice) {
        this->messageDevice = messageDevice;
        return *this;
    }

    SDLMediaPlayer *build() {
        DEBUG = debug;
        SDLMediaPlayer *mediaPlayer = new SDLMediaPlayer();
        mediaPlayer->setMediaSync(mediaSync);
        mediaPlayer->setAudioDevice(audioDevice);
        mediaPlayer->setVideoDevice(videoDevice);
        if (messageDevice == nullptr) {
            this->messageDevice = new MessageDevice();
        }
        mediaPlayer->setMessageDevice(messageDevice);
        this->messageDevice->setMsgListener(mediaPlayer);
        return mediaPlayer;
    }
};


#endif
