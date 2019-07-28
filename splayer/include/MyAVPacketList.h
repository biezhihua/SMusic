//
// Created by biezhihua on 2019-07-16.
//

#ifndef ANDROID_SPLAYER_MYAVPACKETLIST_H
#define ANDROID_SPLAYER_MYAVPACKETLIST_H

#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

class MyAVPacketList {
public:
    AVPacket pkt;
    MyAVPacketList *next;
    int serial;
};

#endif //ANDROID_SPLAYER_MYAVPACKETLIST_H
