#ifndef SPLAYER_AOUT_H
#define SPLAYER_AOUT_H

class Stream;

#include "Log.h"
#include "Mutex.h"
#include "Stream.h"

class Audio {

private:
    Stream *stream = nullptr;
    Mutex *mutex = nullptr;

public:
    Audio();

    virtual ~Audio();

    virtual int create() = 0;

    virtual int destroy() = 0;

    void setStream(Stream *stream);
};


#endif //SPLAYER_AOUT_H
