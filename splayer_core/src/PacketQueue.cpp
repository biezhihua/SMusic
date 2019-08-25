
#include <PacketQueue.h>

#include "PacketQueue.h"


int PacketQueue::init(AVPacket *packet) {

    PacketQueue::flushPacket = packet;

    mutex = new Mutex();

    if (!mutex) {
        ALOGE(PACKET_QUEUE_TAG, "%s create mutex fail", __func__);
        return NEGATIVE(S_NO_MEMORY);
    }

    abortRequest = 1;

    ALOGD(PACKET_QUEUE_TAG, "%s mutex = %p abortRequest = %d",
          __func__,
          mutex,
          abortRequest);

    return POSITIVE;
}

int PacketQueue::destroy() {
    flush();
    delete mutex;
    mutex = nullptr;
    return POSITIVE;
}

int PacketQueue::start() {
    if (mutex) {
        mutex->mutexLock();
        abortRequest = 0;
        putPrivate(flushPacket);
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int PacketQueue::flush() {
    if (mutex) {
        PacketData *packetList, *packetList1;
        mutex->mutexLock();
        for (packetList = firstPacket; packetList; packetList = packetList1) {
            packetList1 = packetList->next;
            av_packet_unref(&packetList->packet);
            av_freep(&packetList);
        }
        lastPacket = nullptr;
        firstPacket = nullptr;
        packetSize = 0;
        memorySize = 0;
        duration = 0;
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int PacketQueue::abort() {
    if (mutex) {
        mutex->mutexLock();
        abortRequest = 1;
        mutex->condSignal();
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int PacketQueue::get(AVPacket *packet, int block, int *serial) {
    // 获取队列首个包数据
    int ret = POSITIVE;
    if (mutex) {
        PacketData *packetData;
        mutex->mutexLock();
        for (;;) {
            if (abortRequest) {
                ret = NEGATIVE(S_ABORT_REQUEST);
                break;
            }
            packetData = firstPacket;
            if (packetData) {
                firstPacket = packetData->next;
                if (!firstPacket) {
                    lastPacket = nullptr;
                }
                packetSize--;
                memorySize -= packetData->packet.size + sizeof(*packetData);
                duration -= packetData->packet.duration;
                *packet = packetData->packet;
                if (serial) {
                    *serial = packetData->serial;
                }
                av_free(packetData);
                ret = POSITIVE;
                break;
            } else if (!block) {
                ret = NEGATIVE(S_NOT_PACKET);
                break;
            } else {
                mutex->condWait();
            }
        }
        mutex->mutexUnLock();
    } else {
        ret = NEGATIVE(S_NULL);
    }
    return ret;
}

int PacketQueue::put(AVPacket *packet) {
    int ret;
    if (mutex) {
        mutex->mutexLock();
        ret = putPrivate(packet);
        mutex->mutexUnLock();
    } else {
        ret = NEGATIVE(S_NULL);
    }
    if (packet != flushPacket && ret < 0) {
        av_packet_unref(packet);
    }
    return ret;
}

int PacketQueue::putNullPacket(int streamIndex) {
    // 入队一个空数据
    AVPacket pkt1, *pkt = &pkt1;
    av_init_packet(pkt);
    pkt->data = nullptr;
    pkt->size = 0;
    pkt->stream_index = streamIndex;
    return put(pkt);
}

int PacketQueue::putPrivate(AVPacket *packet) {
    if (!packet) {
        return NEGATIVE(S_NULL);
    }
    if (abortRequest) {
        return NEGATIVE(S_ABORT_REQUEST);
    }
    PacketData *packetList = new PacketData();
    packetList->packet = *packet;
    packetList->next = nullptr;
    if (packet == flushPacket) {
        serial++;
    }
    packetList->serial = serial;
    if (!lastPacket) {
        firstPacket = packetList;
    } else {
        lastPacket->next = packetList;
    }
    lastPacket = packetList;
    packetSize++;
    memorySize += packetList->packet.size + sizeof(*packetList);
    duration += packetList->packet.duration;
    mutex->condSignal();
    return POSITIVE;
}