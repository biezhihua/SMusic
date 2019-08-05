
#ifndef SPLAYER_MAC_MYAVPACKETLIST_H
#define SPLAYER_MAC_MYAVPACKETLIST_H

extern "C" {
#include <libavformat/avformat.h>
};

class MyAVPacketList {
public:
    AVPacket pkt;
    struct MyAVPacketList *next;
    int serial;
};

#endif
