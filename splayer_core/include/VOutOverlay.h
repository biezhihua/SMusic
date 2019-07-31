#ifndef SPLAYER_VOUTOVERLAY_H
#define SPLAYER_VOUTOVERLAY_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "Log.h"
#include "VOutOverlayOpaque.h"

class VOutOverlay {
private:
    int w;
    int h;
    uint32_t format;
    int planes;
    uint16_t *pPitches;
    uint8_t *pPixels;

    int isPrivate;

    int sarNum;
    int sarDen;

    VOutOverlayOpaque *pOpaque;

public:
    VOutOverlay();

    ~VOutOverlay();
};


#endif //SPLAYER_VOUTOVERLAY_H
