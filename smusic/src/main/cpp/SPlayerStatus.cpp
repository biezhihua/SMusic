//
// Created by biezhihua on 2019/2/11.
//

#include "SPlayerStatus.h"

bool SPlayerStatus::isExit() {
    return state == STATE_EXIT;
}

void SPlayerStatus::changeStateToExit() {
    this->state = STATE_EXIT;
}
