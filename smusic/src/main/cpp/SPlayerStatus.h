//
// Created by biezhihua on 2019/2/11.
//

#ifndef SMUSIC_SPLAYERSTATUS_H
#define SMUSIC_SPLAYERSTATUS_H

#define STATE_START 0
#define STATE_PLAYING 1
#define STATE_PAUSE 2
#define STATE_EXIT 3

class SPlayerStatus {
private:
    int state;

public:
    bool isExit();

    void changeStateToExit();
};


#endif //SMUSIC_SPLAYERSTATUS_H
