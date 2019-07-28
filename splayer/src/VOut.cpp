//
// Created by biezhihua on 2019-06-16.
//

#include "../include/VOut.h"

VOut::VOut() {
    ALOGD(__func__);
    mutex = new Mutex();
}

VOut::~VOut() {
    ALOGD(__func__);
    delete mutex;
}

void VOut::setVOutOpaque(VOutOpaque *vOutOpaque) {
    VOut::vOutOpaque = vOutOpaque;
}

