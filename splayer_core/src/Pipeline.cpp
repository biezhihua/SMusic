#include <VOut.h>
#include "Pipeline.h"

Pipeline::Pipeline() {
    ALOGD(__func__);
}

Pipeline::~Pipeline() {
    ALOGD(__func__);
}

void Pipeline::setVOut(VOut *vOut) {
    Pipeline::vOut = vOut;
}
