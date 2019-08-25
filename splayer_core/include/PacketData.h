#ifndef SPLAYER_CORE_PACKET_DATA_H
#define SPLAYER_CORE_PACKET_DATA_H

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
