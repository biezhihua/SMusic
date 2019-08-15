#ifndef SPLAYER_VOUT_H
#define SPLAYER_VOUT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "Log.h"
#include "Mutex.h"
#include "VOutOverlay.h"

class Surface {

protected:
    Mutex *mutex;

public:
    Surface();

    virtual ~Surface();

    virtual int create() = 0;
    virtual int destroy() = 0;
};


#endif //SPLAYER_VOUT_H
