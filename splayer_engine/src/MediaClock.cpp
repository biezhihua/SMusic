#include "MediaClock.h"

MediaClock::MediaClock() { queueSerial = nullptr; }

MediaClock::~MediaClock() { queueSerial = nullptr; }

void MediaClock::init(int *queueSeekSerial) {
    speed = 1.0;
    paused = 0;
    queueSerial = queueSeekSerial;
    setClock(NAN, -1);
}

double MediaClock::getClock() {
    if (*queueSerial != seekSerial) {
        return NAN;
    }
    if (paused) {
        return pts;
    } else {
        double time = av_gettime_relative() / 1000000.0;
        return ptsDrift + time - (time - lastUpdated) * (1.0 - speed);
    }
}

void MediaClock::setClock(double pts, double time, int serial) {
    this->pts = pts;
    this->lastUpdated = time;
    this->ptsDrift = this->pts - time;
    this->seekSerial = serial;
}

void MediaClock::setClock(double pts, int serial) {
    double time = av_gettime_relative() / 1000000.0;
    setClock(pts, time, serial);
}

void MediaClock::setSpeed(double speed) {
    setClock(getClock(), seekSerial);
    this->speed = speed;
}

void MediaClock::syncToSlave(MediaClock *slave) {
    double clock = getClock();
    double slave_clock = slave->getClock();
    if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD)) {
        setClock(slave_clock, slave->seekSerial);
    }
}

double MediaClock::getSpeed() const { return speed; }

int MediaClock::getPaused() const { return paused; }

void MediaClock::setPaused(int paused) { MediaClock::paused = paused; }

int MediaClock::getSeekSerial() const { return seekSerial; }

double MediaClock::getLastUpdated() const { return lastUpdated; }
