//
// Created by biezhihua on 2019/2/11.
//

#include "SStatus.h"

bool SStatus::isExit() {
    return state == STATE_DESTROY;
}

void SStatus::changeStateToExit() {
    this->state = STATE_DESTROY;
}
