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
};


#endif //SPLAYER_VOUT_H
