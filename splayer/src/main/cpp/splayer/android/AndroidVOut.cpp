#include "AndroidVOut.h"
#include "AndroidVOutOpaque.h"

AndroidVOutSurface::AndroidVOutSurface() {
    ALOGD(__func__);
}

AndroidVOutSurface::~AndroidVOutSurface() {
    ALOGD(__func__);
}

VOutOverlay *AndroidVOutSurface::createOverlay(int width, int height, int frameFormat) {
    return nullptr;
}

int AndroidVOutSurface::displayOverlay(VOutOverlay *overlay) {
    return nullptr;
}

VOutOpaque *AndroidVOutSurface::createOpaque() {
    return new AndroidVOutOpaque();
}
