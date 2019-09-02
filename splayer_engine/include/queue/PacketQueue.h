#ifndef PACKETQUEUE_H
#define PACKETQUEUE_H

#include <queue>
#include <common/Mutex.h>
#include <common/Condition.h>

extern "C" {
#include <libavcodec/avcodec.h>
};

typedef struct PacketList {
    AVPacket pkt;
    struct PacketList *next;
} PacketList;

/**
 * 备注：这里不用std::queue是为了方便计算队列占用内存和队列的时长，在解码的时候要用到
 */
class PacketQueue {
public:
    PacketQueue();

    virtual ~PacketQueue();

    // 入队数据包
    int pushPacket(AVPacket *pkt);

    // 入队空数据包
    int pushNullPacket(int stream_index);

    // 刷新
    void flush();

    // 终止
    void abort();

    // 开始
    void start();

    // 获取数据包
    int getPacket(AVPacket *pkt);

    // 获取数据包
    int getPacket(AVPacket *pkt, int block);

    int getPacketSize();

    int getSize();

    int64_t getDuration();

    int isAbort();

private:
    int put(AVPacket *pkt);

private:

    Mutex mutex;

    Condition condition;

    PacketList *firstPacket;
    PacketList *lastPacket;

    /// 包数量，也就是队列元素数量
    int packetSize;

    /// 占用内存大小
    int memorySize;

    /// 持续时长
    int64_t duration;

    /// 用户退出请求标志
    bool abortRequest;
};


#endif //PACKETQUEUE_H
