#include "Clock.h"

int Clock::initClock(int *queueSerial) {
    Clock::speed = 1.0F;
    Clock::paused = 0;
    Clock::queueSerial = queueSerial;
    setClock(NAN, -1);
    return POSITIVE;
}

int Clock::setClock(double pts, int serial) {
    double time = av_gettime_relative() / 1000000.0;
    setClockAt(pts, serial, time);
    return POSITIVE;
}

int Clock::setClockAt(double pts, int serial, double time) {
    Clock::pts = pts;
    Clock::ptsDrift = pts - time;
    Clock::lastUpdated = time;
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
