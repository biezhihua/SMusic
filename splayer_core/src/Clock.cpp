#include "Clock.h"

int Clock::init(int *queueSerial) {
    Clock::speed = 1.0F;
    Clock::paused = 0;
    Clock::queueSerial = queueSerial;
    setClock(NAN, -1);

    ALOGD(CLOCK_TAG, "%s speed = %f speed = %d queueSerial = %d pts = %lf ptsDrift = %lf lastUpdatedTime = %lf",
          __func__, speed, paused, *queueSerial, pts, ptsDrift, lastUpdatedTime);
    return POSITIVE;
}

/// 设置时钟
int Clock::setClock(double pts, int serial) {
    double time = av_gettime_relative() * 1.0F / AV_TIME_BASE;
    setClockAt(pts, serial, time);
    return POSITIVE;
}

/// 设置时钟
int Clock::setClockAt(double pts, int serial, double time) {
    Clock::pts = pts;
    Clock::ptsDrift = pts - time;
    Clock::lastUpdatedTime = time;
    Clock::seekSerial = serial;
    return POSITIVE;
}

/// 获取时钟
double Clock::getClock() {
    if (*queueSerial != seekSerial) {
        return NAN;
    }
    if (paused) {
        return pts;
    } else {
        double time = av_gettime_relative() * 1.0F / AV_TIME_BASE;
        return ptsDrift + time - (time - lastUpdatedTime) * (1.0 - speed);
    }
}

/// 设置时钟速度
void Clock::setClockSpeed(double speed) {
    setClock(getClock(), seekSerial);
    Clock::speed = speed;
}

/// 同步从属时钟
void Clock::syncClockToSlave(Clock *slave) {
    double clock = getClock();
    double slaveClock = slave->getClock();
    if (!isnan(slaveClock) && (isnan(clock) || fabs(clock - slaveClock) > NO_SYNC_THRESHOLD)) {
        setClock(slaveClock, slave->seekSerial);
    }
}