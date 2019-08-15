#ifndef SPLAYER_AOUT_H
#define SPLAYER_AOUT_H

#include "Log.h"
#include "Mutex.h"

class Audio {

private:
    Mutex *mutex = nullptr;

public:
    Audio();

    virtual ~Audio();

    virtual int create() = 0;

    virtual int destroy() = 0;

};


#endif //SPLAYER_AOUT_H
