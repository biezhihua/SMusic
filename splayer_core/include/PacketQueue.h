#ifndef SPLAYER_MAC_PACKETQUEUE_H
#define SPLAYER_MAC_PACKETQUEUE_H

#include "Mutex.h"

class PacketQueue {

public:
    //MyAVPacketList *first_pkt, *last_pkt;
    int nbPackets;
    int size;
    int64_t duration;
    int abortRequest;
    int serial;
    Mutex *mutex;
};


#endif //SPLAYER_MAC_PACKETQUEUE_H
