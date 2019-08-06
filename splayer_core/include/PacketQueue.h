#ifndef SPLAYER_MAC_PACKETQUEUE_H
#define SPLAYER_MAC_PACKETQUEUE_H

#include "Mutex.h"
#include "MyAVPacketList.h"
#include "Error.h"
#include "Log.h"

class PacketQueue {

private:
    AVPacket *flushPacket;
    // 队首、队尾指针
    MyAVPacketList *firstPacketList, *lastPacketList;
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

    int packetQueueInit(AVPacket *pPacket);

    int packetQueueStart();

    int packetQueueAbort();

    int packetQueuePut(AVPacket *packet);

    int packetQueuePutNullPacket(int streamIndex);

    int packetQueuePutPrivate(AVPacket *packet);

    int packetQueueDestroy();

    int packetQueueFlush();

    int packetQueueGet(AVPacket *packet, int block, int *serial);

};


#endif //SPLAYER_MAC_PACKETQUEUE_H
