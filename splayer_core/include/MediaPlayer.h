#ifndef SPLAYER_CORE_MEDIASPLAYER_H
#define SPLAYER_CORE_MEDIASPLAYER_H

class MessageQueue;

class Event;

class Message;

#include "Log.h"
#include "Error.h"
#include "Mutex.h"
#include "Stream.h"
#include "Stream.h"
#include "State.h"
#include "Thread.h"
#include "Options.h"
#include "MessageQueue.h"
#include "Audio.h"
#include "Surface.h"
#include "Event.h"
#include "Message.h"

#define MEDIA_PLAYER_TAG "MediaPlayer"

class MediaPlayer {

private:
    State *state = nullptr;
    Mutex *mutex = nullptr;

    MessageQueue *msgQueue = nullptr;
    Thread *msgThread = nullptr;
    char *dataSource = nullptr;

    Options *options = nullptr;
    Event *event = nullptr;
    Message *message = nullptr;
    Stream *stream = nullptr;
    Audio *audio = nullptr;
    Surface *surface = nullptr;

    bool quit = false;

public:
    class Builder;

    MediaPlayer();

    ~MediaPlayer();

    int create();

    int start();

    int stop();

    int pause();

    int destroy();

    int reset();

    int setDataSource(const char *url);

    int prepareAsync();

    void setMessage(Message *message);

    void setEvent(Event *event);

    void setStream(Stream *stream);

    void setAudio(Audio *audio);

    void setSurface(Surface *surface);

    void setOptions(Options *options);

    int messageLoop();

    int eventLoop();

private:

    int notifyMsg(int what);

    int notifyMsg(int what, int arg1);

    int notifyMsg(int what, int arg1, int arg2);

    int removeMsg(int what);

    int getMsg(Msg *pMessage, bool block);

    int prepareOptions();

    int prepareMsgQueue();

    int prepareSurface();

    int prepareStream();

    int prepareAudio();

    int prepareEvent();

};

class MediaPlayer::Builder {
private:
    Message *iMessage;
    Event *iEvent;
    Audio *iAudio;
    Surface *iSurface;
    Stream *iStream;
    Options *iOptions;

public:
    Builder &withMessage(Message *message) {
        iMessage = message;
        return *this;
    }

    Builder &withEvent(Event *event) {
        iEvent = event;
        return *this;
    }

    Builder &withAudio(Audio *audio) {
        iAudio = audio;
        return *this;
    }

    Builder &withSurface(Surface *surface) {
        iSurface = surface;
        return *this;
    }

    Builder &withStream(Stream *stream) {
        iStream = stream;
        return *this;
    }

    Builder &withOptions(Options *options) {
        iOptions = options;
        return *this;
    }

    MediaPlayer *build() {
        MediaPlayer *mediaPlayer = new MediaPlayer();
        mediaPlayer->setOptions(iOptions);
        mediaPlayer->setMessage(iMessage);
        mediaPlayer->setEvent(iEvent);
        mediaPlayer->setAudio(iAudio);
        mediaPlayer->setSurface(iSurface);
        mediaPlayer->setStream(iStream);
        return mediaPlayer;
    }
};


#endif
