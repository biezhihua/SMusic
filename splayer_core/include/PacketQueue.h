#ifndef SPLAYER_MAC_PACKETQUEUE_H
#define SPLAYER_MAC_PACKETQUEUE_H

#include "Mutex.h"
#include "MyAVPacketList.h"

class PacketQueue {

public:
    MyAVPacketList *firstPacketList, *lastPacketList;
    int nbPackets;
    int size;
    int64_t duration;
    int abortRequest;
    int serial;
    Mutex *mutex;
};


#endif //SPLAYER_MAC_PACKETQUEUE_H
