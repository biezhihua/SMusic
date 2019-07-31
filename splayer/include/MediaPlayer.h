#ifndef SPLAYER_MEDIASPLAYER_H
#define SPLAYER_MEDIASPLAYER_H

#include "Log.h"
#include "Error.h"
#include "Mutex.h"
#include "FFPlay.h"
#include "RefCount.h"
#include "Pipeline.h"
#include "State.h"
#include "Thread.h"

class MediaPlayer {

protected:
    State *state = nullptr;
    Mutex *mutex = nullptr;
    FFPlay *play = nullptr;
    static RefCount refCount;
    char *dataSource = nullptr;
    Thread *msgThread = nullptr;

protected:
    void notifyMsg(int what);

    void notifyMsg(int what, int arg1);

    void notifyMsg(int what, int arg1, int arg2);

    void removeMsg(int what);

protected:
    virtual AOut *createAOut() = 0;

    virtual VOut *createSurface() = 0;

    virtual Pipeline *createPipeline() = 0;

    MessageQueue *getMsgQueue();

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

    virtual int messageLoop() = 0;
};


#endif //SPLAYER_MEDIASPLAYER_H
