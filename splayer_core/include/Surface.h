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

extern "C" {
#include <libavutil/rational.h>
}

class Surface {

protected:
    FFPlay *play = nullptr;
    Mutex *mutex = nullptr;

public:
    Surface();

    virtual ~Surface();

    virtual int create() = 0;

    virtual int destroy() = 0;

    virtual void setWindowSize(int width, int height, AVRational rational);

    void setPlay(FFPlay *play);

    virtual void openWindow(int width, int height);
};


#endif //SPLAYER_VOUT_H
