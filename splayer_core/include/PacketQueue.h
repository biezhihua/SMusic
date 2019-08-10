#ifndef SPLAYER_MAC_PACKETQUEUE_H
#define SPLAYER_MAC_PACKETQUEUE_H

#include "Mutex.h"
#include "PacketData.h"
#include "Error.h"
#include "Log.h"

#define PACKET_QUEUE_TAG "PacketQueue"

class PacketQueue {

private:
    AVPacket *flushPacket;
    PacketData *firstPacket, *lastPacket;// 队首、队尾指针
    Mutex *mutex;
public:
    // 占用内存大小
    int memorySize;
    // 序列，作拖动时用，作为区分前后帧序列
    int serial;
    // 包数量，也就是队列元素数量
    int packetSize;
    int64_t duration;
    // 用户退出请求标志
    int abortRequest;

public:

    int init(AVPacket *pPacket);

    int start();

    int abort();

    int put(AVPacket *packet);

    int putNullPacket(int streamIndex);

    int putPrivate(AVPacket *packet);

    int destroy();

    int flush();

    int get(AVPacket *packet, int block, int *serial);

};


#endif //SPLAYER_MAC_PACKETQUEUE_H
