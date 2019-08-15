#ifndef SPLAYER_AOUT_H
#define SPLAYER_AOUT_H

class FFPlay;

#include "Log.h"
#include "Mutex.h"
#include "FFPlay.h"

class Audio {

private:
    FFPlay *play = nullptr;
    Mutex *mutex = nullptr;

public:
    Audio();

    virtual ~Audio();

    virtual int create() = 0;

    virtual int destroy() = 0;

    void setPlay(FFPlay *play);
};


#endif //SPLAYER_AOUT_H
