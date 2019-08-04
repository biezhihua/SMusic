#ifndef SPLAYER_MAC1_CLOCK_H
#define SPLAYER_MAC1_CLOCK_H

#include "Error.h"

extern "C" {
#include <libavutil/time.h>
#include <libavutil/mathematics.h>
};


class Clock {
public:

    /**
     * clock base
     */
    double pts;

    /**
     * clock base minus time at which we updated the clock
     */
    double ptsDrift;

    double lastUpdated;

    double speed;
    /**
     * clock is based on a packet with this serial
     */
    int serial;

    int paused;
    /**
     * pointer to the current packet queue serial, used for obsolete clock detection
     */
    int *queueSerial;

public:
    int initClock(int *pQueueSerial);

    int setClock(double pts, int serial);

    int setClockAt(double pts, int serial, double time);

    double getClock();


};


#endif //SPLAYER_MAC_CLOCK_H
