#ifndef SPLAYER_MEDIASPLAYER_H
#define SPLAYER_MEDIASPLAYER_H

class MessageQueue;
class Event;

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

#define MEDIA_PLAYER_TAG "MediaPlayer"

class MediaPlayer {

protected:
    State *state = nullptr;
    Mutex *mutex = nullptr;
    MessageQueue *msgQueue = nullptr;
    Stream *stream = nullptr;
    Audio *audio = nullptr;
    Surface *surface = nullptr;
    Event *event = nullptr;
    char *dataSource = nullptr;
    Thread *msgThread = nullptr;
    Options *options = nullptr;

protected:

    int notifyMsg(int what);

    int notifyMsg(int what, int arg1);

    int notifyMsg(int what, int arg1, int arg2);

    int removeMsg(int what);

    int getMsg(Message *pMessage, bool block);

public:

    MediaPlayer();

    virtual ~MediaPlayer();

    virtual int create();

    virtual int start();

    virtual int stop();

    virtual int pause();

    virtual int destroy();

    virtual int reset();

    virtual int setDataSource(const char *url);

    virtual int prepareAsync();

public:

    virtual Event *createEvent() = 0;

    virtual Audio *createAudio() = 0;

    virtual Surface *createSurface() = 0;

    virtual Stream *createStream() = 0;

    virtual int messageLoop() = 0;

    virtual Options *createOptions() const;

private:

    int prepareOptions();

    int prepareMsgQueue();

    int prepareSurface();

    int prepareStream();

    int prepareAudio();

    int prepareEvent();
};


#endif
