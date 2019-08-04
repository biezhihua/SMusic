#include "Clock.h"

int Clock::initClock(int *pQueueSerial) {
    speed = 1.0F;
    paused = 0;
    queueSerial = pQueueSerial;
    setClock(NAN, -1);
    return POSITIVE;
}

int Clock::setClock(double pts, int serial) {
    double time = av_gettime_relative() / 1000000.0;
    setClockAt(pts, serial, time);
    return POSITIVE;
}

int Clock::setClockAt(double pts, int serial, double time) {
    pts = pts;
    lastUpdated = time;
    ptsDrift = pts - time;
    Clock::serial = serial;
    return POSITIVE;
}

double Clock::getClock() {
    if (*queueSerial != serial) {
        return NAN;
    }
    if (paused) {
        return pts;
    } else {
        double time = av_gettime_relative() / 1000000.0;
        return ptsDrift + time - (time - lastUpdated) * (1.0 - speed);
    }
}
