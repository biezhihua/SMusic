#ifndef SPLAYER_MEDIASPLAYER_H
#define SPLAYER_MEDIASPLAYER_H

#include "Log.h"
#include "Common.h"
#include "Mutex.h"
#include "FFPlay.h"
#include "RefCount.h"
#include "Pipeline.h"
#include "State.h"
#include "Thread.h"
#include <libavutil/time.h>

/**
 * virtual
 */
class MediaPlayer {

protected:
    State *pState = nullptr;
    Mutex *pMutex = nullptr;
    FFPlay *pPlay = nullptr;
    static RefCount refCount;
    char *pDataSource = nullptr;
    Thread *pMsgThread = nullptr;

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
