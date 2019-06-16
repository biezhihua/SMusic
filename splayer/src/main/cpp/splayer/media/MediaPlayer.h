#ifndef SPLAYER_MEDIAPLAYER_H
#define SPLAYER_MEDIAPLAYER_H

#include "Log.h"
#include "Common.h"
#include "Mutex.h"
#include "FFPlay.h"
#include "RefCount.h"
#include "Pipeline.h"
#include "State.h"
#include "Thread.h"

class MediaPlayer {

private:
    State *pState = nullptr;
    Mutex *pMutex = nullptr;
    FFPlay *pPlay = nullptr;
    static RefCount refCount;
    char *pDataSource = nullptr;
    Thread *pMsgThread = nullptr;

private:
    void notifyMsg1(int what);

    void notifyMsg2(int what, int arg1);

    void notifyMsg3(int what, int arg1, int arg2);

    void removeMsg(int what);

protected:
    virtual AOut *createAOut() = 0;

    virtual VOut *createSurface() = 0;

    virtual Pipeline *createPipeline() = 0;

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

    int messageLoop();
};


#endif //SPLAYER_MEDIAPLAYER_H
