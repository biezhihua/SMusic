#ifndef SPLAYER_CORE_PACKET_QUEUE_H
#define SPLAYER_CORE_PACKET_QUEUE_H

#include "Mutex.h"
#include "PacketData.h"
#include "Error.h"
#include "Log.h"

#define PACKET_QUEUE_TAG "PacketQueue"

class PacketQueue {

private:

    AVPacket *flushPacket;

    /// 队首指针
    PacketData *firstPacket;

    /// 队尾指针
    PacketData *lastPacket;

    Mutex *mutex;
public:

    /// 占用内存大小
    int memorySize;

    /// 序列，seek时使用，作为区分前后帧序列
    int seekSerial;

    /// 包数量，也就是队列元素数量
    int packetSize;

    /// 持续时长
    int64_t duration;

    /// 用户退出请求标志
    int abortRequest;

public:

    int init(AVPacket *packet);

    int start();

    int abort();

    int put(AVPacket *packet);

    int putNullPacket(int streamIndex);

    int putPrivate(AVPacket *packet);

    int destroy();

    int flush();

    int get(AVPacket *packet, int block, int *serial);

};

#endif //SPLAYER_CORE_PACKET_QUEUE_H
