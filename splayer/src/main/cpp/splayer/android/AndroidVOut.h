#ifndef SPLAYER_ANDROIDSURFACE_H
#define SPLAYER_ANDROIDSURFACE_H


#include "../media/VOut.h"
#include "../media/Log.h"
#include "AndroidVOutOpaque.h"

class AndroidVOutSurface : public VOut {

public:
    AndroidVOutSurface();

    virtual ~AndroidVOutSurface();

    VOutOverlay *createOverlay(int width, int height, int frameFormat) override;

    int displayOverlay(VOutOverlay *overlay) override;

    VOutOpaque *createOpaque() override;

    void free() override;

};


#endif //SPLAYER_ANDROIDSURFACE_H
