#ifndef SDL_MEDIAPLAYER_H
#define SDL_MEDIAPLAYER_H

#include <MediaPlayer.h>
#include <MessageCenter.h>
#include <IMessageListener.h>
#include <MediaSync.h>
#include <SDL.h>
#include <SDLVideoDevice.h>

#define MSG_REQUEST_SEEK_SDL 29000
#define MSG_REQUEST_PLAY_OR_PAUSE 29001

class SDLMediaPlayer : public MediaPlayer {

    const char *const TAG = "[MP][SDL][MediaPlayer]";

private:

    bool quit = false;

    int64_t lastMouseLeftClick = 0;

    void doKeySystem(const SDL_Event &event);

    bool isNotHaveWindow();

    bool isQuitKey(const SDL_Event &event);

public:

    class Builder;

    int eventLoop();

    void doWindowEvent(const SDL_Event &event);

    void showCursor();

    void hideCursor();

    int isFullScreenClick();

    void doExit();

    void doSeek(int increment);

    int destroy() override;

    void resetRemainingTime();

    void refreshVideo();

    void toggleFullScreen();
};

class SDLMediaPlayer::Builder {
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

    SDLMediaPlayer *build() {
        DEBUG = debug;
        SDLMediaPlayer *mediaPlayer = new SDLMediaPlayer();
        mediaPlayer->setMediaSync(mediaSync);
        mediaPlayer->setAudioDevice(audioDevice);
        mediaPlayer->setVideoDevice(videoDevice);
        mediaPlayer->setMessageListener(messageListener);
        return mediaPlayer;
    }
};

#endif
