#include "PacketQueue.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "OCDFAInspection"

int PacketQueue::packetQueueInit(AVPacket *pFlushPacket) {

    flushPacket = pFlushPacket;

    mutex = new Mutex();

    if (!mutex) {
        ALOGE("%s create mutex fail", __func__);
        return NEGATIVE(S_NOT_MEMORY);
    }

    abortRequest = 1;

    ALOGI("%s mutex=%p abortRequest=%d",
          __func__,
          mutex,
          abortRequest);

    return POSITIVE;
}

int PacketQueue::packetQueueDestroy() {
    packetQueueFlush();
    delete mutex;
    mutex = nullptr;
    return POSITIVE;
}

int PacketQueue::packetQueueStart() {
    if (mutex) {
        mutex->mutexLock();
        abortRequest = 0;
        packetQueuePutPrivate(flushPacket);
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int PacketQueue::packetQueueFlush() {
    if (mutex) {
        MyAVPacketList *packetList, *packetList1;
        mutex->mutexLock();
        for (packetList = firstPacketList; packetList; packetList = packetList1) {
            packetList1 = packetList->next;
            av_packet_unref(&packetList->packet);
            av_freep(&packetList);
        }
        lastPacketList = nullptr;
        firstPacketList = nullptr;
        packetSize = 0;
        memorySize = 0;
        duration = 0;
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int PacketQueue::packetQueueAbort() {
    if (mutex) {
        mutex->mutexLock();
        abortRequest = 1;
        mutex->condSignal();
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int PacketQueue::packetQueueGet(AVPacket *packet, int block, int *serial) {
    int ret = 0;
    if (mutex) {
        MyAVPacketList *packetList;
        mutex->mutexLock();
        for (;;) {
            if (abortRequest) {
                ret = NEGATIVE(S_NOT_ABORT_REQUEST);
                break;
            }
            packetList = firstPacketList;
            if (packetList) {
                firstPacketList = packetList->next;
                if (!firstPacketList) {
                    lastPacketList = nullptr;
                }
                packetSize--;
                memorySize -= packetList->packet.size + sizeof(*packetList);
                duration -= packetList->packet.duration;
                *packet = packetList->packet;
                if (serial) {
                    *serial = packetList->serial;
                }
                av_free(packetList);
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

int PacketQueue::packetQueuePut(AVPacket *packet) {
    int ret;
    if (mutex) {
        mutex->mutexLock();
        ret = packetQueuePutPrivate(packet);
        mutex->mutexUnLock();
    } else {
        ret = NEGATIVE(S_NULL);
    }
    if (packet != flushPacket && ret < 0) {
        av_packet_unref(packet);
    }
    return ret;
}

int PacketQueue::packetQueuePutNullPacket(int streamIndex) {
    AVPacket pkt1, *pkt = &pkt1;
    av_init_packet(pkt);
    pkt->data = nullptr;
    pkt->size = 0;
    pkt->stream_index = streamIndex;
    return packetQueuePut(pkt);
}

int PacketQueue::packetQueuePutPrivate(AVPacket *packet) {
    if (!packet) {
        return NEGATIVE(S_NULL);
    }
    if (abortRequest) {
        return NEGATIVE(S_NOT_ABORT_REQUEST);
    }
    auto *packetList = new MyAVPacketList();
    if (!packetList) {
        return NEGATIVE(S_NOT_MEMORY);
    }
    packetList->packet = *packet;
    packetList->next = nullptr;
    if (packet == flushPacket) {
        serial++;
    }
    packetList->serial = serial;
    if (!lastPacketList) {
        firstPacketList = packetList;
    } else {
        lastPacketList->next = packetList;
    }
    lastPacketList = packetList;
    packetSize++;
    memorySize += packetList->packet.size + sizeof(*packetList);
    duration += packetList->packet.duration;
    /* XXX: should duplicate packet data in DV case */
    mutex->condSignal();
    return POSITIVE;
}

#pragma clang diagnostic pop