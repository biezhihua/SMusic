#include "VOut.h"

VOut::VOut() {
    ALOGD(__func__);
    mutex = new Mutex();
}

VOut::~VOut() {
    ALOGD(__func__);
    delete mutex;
}


