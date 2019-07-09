#ifndef SPLAYER_AOUT_H
#define SPLAYER_AOUT_H

#include "Log.h"
#include "Mutex.h"

class AOut {

private:
    Mutex *pMutex = nullptr;

public:
    AOut();

    virtual ~AOut();

    virtual int open() = 0;

    virtual void pause() = 0;

    virtual void flush() = 0;

    virtual void close() = 0;

    virtual void free() = 0;

};


#endif //SPLAYER_AOUT_H
