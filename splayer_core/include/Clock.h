#ifndef SPLAYER_CORE_CLOCK_H
#define SPLAYER_CORE_CLOCK_H

#include "Error.h"
#include "Log.h"
#include "Define.h"

extern "C" {
#include <libavutil/time.h>
#include <libavutil/mathematics.h>
#include <libavutil/avutil.h>
};

#define CLOCK_TAG  "Clock"

/// 时钟
class Clock {
public:

    /// 展示时间
    double pts;

    /// 偏移时间，展示时间减去更新的时间
    double ptsDrift;

    /// 更新时间
    double updatedTime;

    /// 速度
    double speed;

    /// 时钟是基于串行包的
    int serial;

    /// 是否暂停
    int paused;

    /// 指向当前包队列串行的指针，用于过时的时钟检测
    int *queueSerial;

public:
    int init(int *queueSerial);

    int setClock(double pts, int serial);

    int setClockAt(double pts, int serial, double time);

    double getClock();

    void setClockSpeed(double speed);

    void syncClockToSlave(Clock *slave);

};


#endif //SPLAYER_MAC_CLOCK_H
