#ifndef SPLAYER_MEDIAPLAYER_H
#define SPLAYER_MEDIAPLAYER_H

#include "Log.h"
#include "Common.h"
#include "Mutex.h"
#include "FFPlay.h"
#include "RefCount.h"
#include "Pipeline.h"

class MediaPlayer {

private:
    Mutex *pMutex = nullptr;
    FFPlay *pPlay = nullptr;
    static RefCount refCount;

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


};


#endif //SPLAYER_MEDIAPLAYER_H
