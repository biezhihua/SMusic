//
// Created by biezhihua on 2019/2/11.
//

#ifndef SMUSIC_SPLAYERSTATUS_H
#define SMUSIC_SPLAYERSTATUS_H

#define STATE_CREATE 0
#define STATE_START 1
#define STATE_RESUME 2
#define STATE_PAUSE 3
#define STATE_STOP 4
#define STATE_DESTROY 5

class SStatus {
private:
    int state;

public:
    bool isExit();

    void changeStateToExit();
};


#endif //SMUSIC_SPLAYERSTATUS_H
