
#ifndef SPLAYER_MAC_MYAVPACKETLIST_H
#define SPLAYER_MAC_MYAVPACKETLIST_H

extern "C" {
#include <libavformat/avformat.h>
};

class PacketData {
public:
    AVPacket packet;
    PacketData *next;
    int serial;
};

#endif
