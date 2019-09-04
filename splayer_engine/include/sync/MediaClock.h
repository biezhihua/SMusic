#ifndef MEDIACLOCK_H
#define MEDIACLOCK_H

#include <math.h>
#include <player/PlayerState.h>

extern "C" {
#include <libavutil/time.h>
};

class MediaClock {

public:
    MediaClock();

    virtual ~MediaClock();

    void init(int *queueSeekSerial);

    // 获取时钟
    double getClock();

    // 设置时钟
    void setClock(double pts, double time, int serial);

    // 设置时钟
    void setClock(double pts, int serial);

    // 设置速度
    void setSpeed(double speed);

    // 同步到从属时钟
    void syncToSlave(MediaClock *slave);

    // 获取时钟速度
    double getSpeed() const;

private:

    /// 时钟基准
    double pts;

    /// 更新时钟的差值
    double ptsDrift;

    /// 上一次更新的时间
    double lastUpdated;

    /// 速度
    double speed;

    /// 停止标志
    int paused;

    /// 时钟基于使用该序列的包
    int seekSerial;

    /// 指向当前包队列串行的指针，用于过时的时钟检测
    int *queueSerial;
};


#endif //MEDIACLOCK_H
