//
// Created by biezhihua on 2019-07-16.
//

#ifndef ANDROID_SPLAYER_CLOCK_H
#define ANDROID_SPLAYER_CLOCK_H

class Clock {
public:
    double pts;           /* clock base */
    double pts_drift;     /* clock base minus time at which we updated the clock */
    double last_updated;
    double speed;
    int serial;           /* clock is based on a packet with this serial */
    int paused;
    int *queue_serial;    /* pointer to the current packet queue serial, used for obsolete clock detection */
};


#endif //ANDROID_SPLAYER_CLOCK_H
