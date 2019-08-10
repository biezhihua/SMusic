#include <VOut.h>
#include "Pipeline.h"

Pipeline::Pipeline() {
}

Pipeline::~Pipeline() {
}

void Pipeline::setVOut(VOut *vOut) {
    Pipeline::vOut = vOut;
}
