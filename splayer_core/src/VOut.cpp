#include "VOut.h"

VOut::VOut() {
    mutex = new Mutex();
}

VOut::~VOut() {
    delete mutex;
}


