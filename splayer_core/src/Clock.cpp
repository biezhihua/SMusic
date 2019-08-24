#include "Clock.h"

int Clock::init(int *queueSerial) {
    Clock::speed = 1.0F;
    Clock::paused = 0;
    Clock::queueSerial = queueSerial;
    setClock(NAN, -1);

    ALOGD(CLOCK_TAG, "%s speed = %f speed = %d queueSerial = %d pts = %lf ptsDrift = %lf lastUpdatedTime = %lf", __func__, speed, paused, *queueSerial, pts, ptsDrift, lastUpdatedTime);
    return POSITIVE;
}

int Clock::setClock(double pts, int serial) {
    double time = av_gettime_relative() * 1.0F / AV_TIME_BASE;
    setClockAt(pts, serial, time);
    return POSITIVE;
}

int Clock::setClockAt(double pts, int serial, double time) {
    Clock::pts = pts;
    Clock::ptsDrift = pts - time;
    Clock::lastUpdatedTime = time;
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
        double time = av_gettime_relative() * 1.0F / AV_TIME_BASE;
        return ptsDrift + time - (time - lastUpdatedTime) * (1.0 - speed);
    }
}

void Clock::setClockSpeed(double speed) {
    setClock(getClock(), serial);
    Clock::speed = speed;
}

void Clock::syncClockToSlave(Clock *slave) {
    double clock = getClock();
    double slaveClock = slave->getClock();
    if (!isnan(slaveClock) && (isnan(clock) || fabs(clock - slaveClock) > NOSYNC_THRESHOLD)) {
        setClock(slaveClock, slave->serial);
    }
}