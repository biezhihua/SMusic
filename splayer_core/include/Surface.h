#ifndef SPLAYER_VOUT_H
#define SPLAYER_VOUT_H

class FFPlay;

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "Log.h"
#include "FFPlay.h"
#include "Mutex.h"
#include "VOutOverlay.h"

class Surface {

protected:
    FFPlay *play;
    Mutex *mutex;

public:
    Surface();

    virtual ~Surface();

    virtual int create() = 0;

    virtual int destroy() = 0;

    void setPlay(FFPlay *play);
};


#endif //SPLAYER_VOUT_H
