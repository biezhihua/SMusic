#ifndef SPLAYER_VOUT_H
#define SPLAYER_VOUT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "Log.h"
#include "Mutex.h"
#include "VOutOverlay.h"

class VOut {

protected:
    Mutex *mutex;

public:
    VOut();

    virtual ~VOut();
};


#endif //SPLAYER_VOUT_H
