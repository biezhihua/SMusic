#ifndef SPLAYER_MAC_PACKETQUEUE_H
#define SPLAYER_MAC_PACKETQUEUE_H

#include "Mutex.h"
#include "MyAVPacketList.h"
#include "Error.h"
#include "Log.h"

class PacketQueue {

private:
    AVPacket *flushPacket;
    MyAVPacketList *firstPacketList, *lastPacketList;

    Mutex *mutex;

public:
    int size;
    int serial;
    int nbPackets;
    int64_t duration;
    int abortRequest;

public:

    int packetQueueInit(AVPacket *pPacket);

    int packetQueueStart();

    int packetQueueAbort();

    int packetQueuePut(AVPacket *pPacket);

    int packetQueuePutNullPacket(int streamIndex);

    int packetQueuePutPrivate(AVPacket *avPacket);

    int packetQueueDestroy();

    int packetQueueFlush();

    int packetQueueGet(AVPacket *pPacket, int block, int *serial);

};


#endif //SPLAYER_MAC_PACKETQUEUE_H
