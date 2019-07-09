//
// Created by biezhihua on 2019-06-16.
//

#ifndef SPLAYER_VOUT_H
#define SPLAYER_VOUT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "Log.h"
#include "Mutex.h"
#include "VOutOpaque.h"
#include "VOutOverlayOpaque.h"
#include "VOutOverlay.h"

class VOut {

protected:
    Mutex *pMutex;
    VOutOpaque *pVOutOpaque;
    uint32_t overlayFormat;

public:
    VOut();

    virtual ~VOut();

    virtual VOutOpaque *createOpaque() = 0;

    virtual VOutOverlay *createOverlay(int width, int height, int frameFormat) = 0;

    virtual int displayOverlay(VOutOverlay *overlay) = 0;

    virtual void free() = 0;

    void setVOutOpaque(VOutOpaque *vOutOpaque);
};


#endif //SPLAYER_VOUT_H
