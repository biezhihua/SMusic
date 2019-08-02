#include "MacVOut.h"
#include "MacVOutOpaque.h"

VOutOpaque *MacVOut::createOpaque() {
    return new MacVOutOpaque();
}

VOutOverlay *MacVOut::createOverlay(int width, int height, int frameFormat) {
    return nullptr;
}

int MacVOut::displayOverlay(VOutOverlay *overlay) {
    return 0;
}

void MacVOut::free() {

}
