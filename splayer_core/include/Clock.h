#ifndef SPLAYER_MAC_CLOCK_H
#define SPLAYER_MAC_CLOCK_H

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
};


#endif //SPLAYER_MAC_CLOCK_H
