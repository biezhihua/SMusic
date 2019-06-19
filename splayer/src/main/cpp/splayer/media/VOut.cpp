//
// Created by biezhihua on 2019-06-16.
//

#include "VOut.h"

VOut::VOut() {
    ALOGD(__func__);
    pMutex = new Mutex();
}

VOut::~VOut() {
    ALOGD(__func__);
    delete pMutex;
}

void VOut::setVOutOpaque(VOutOpaque *vOutOpaque) {
    VOut::pVOutOpaque = vOutOpaque;
}
