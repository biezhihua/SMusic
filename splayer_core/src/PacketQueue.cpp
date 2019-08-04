

#include "PacketQueue.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "OCDFAInspection"

int PacketQueue::packetQueueFlush() {
    if (mutex) {
        MyAVPacketList *pkt, *pkt1;
        mutex->mutexLock();
        for (pkt = firstPacketList; pkt; pkt = pkt1) {
            pkt1 = pkt->next;
            av_packet_unref(&pkt->pkt);
            av_freep(&pkt);
        }
        lastPacketList = nullptr;
        firstPacketList = nullptr;
        nbPackets = 0;
        size = 0;
        duration = 0;
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int PacketQueue::packetQueueStart() {
    if (mutex) {
        mutex->mutexLock();
        abortRequest = 1;
        mutex->condSignal();
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int PacketQueue::packetQueueAbort() {
    if (mutex) {
        mutex->mutexLock();
        abortRequest = 0;
        packetQueuePutPrivate(flushPacket);
        mutex->mutexUnLock();
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int PacketQueue::packetQueueGet(AVPacket *pPacket, int block, int *serial) {
    int ret = 0;
    if (mutex) {
        MyAVPacketList *pkt1;
        mutex->mutexLock();
        for (;;) {
            if (abortRequest) {
                ret = NEGATIVE(S_NOT_ABORT_REQUEST);
                break;
            }
            pkt1 = firstPacketList;
            if (pkt1) {
                firstPacketList = pkt1->next;
                if (!firstPacketList) {
                    lastPacketList = nullptr;
                }
                nbPackets--;
                size--;
                duration -= pkt1->pkt.duration;
                *pPacket = pkt1->pkt;
                if (serial) {
                    *serial = pkt1->serial;
                }
                av_free(pkt1);
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

int PacketQueue::packetQueuePut(AVPacket *pPacket) {
    int ret;
    if (mutex) {
        mutex->mutexLock();
        ret = packetQueuePutPrivate(pPacket);
        mutex->mutexUnLock();
    } else {
        ret = NEGATIVE(S_NULL);
    }
    if (pPacket != flushPacket && ret < 0) {
        av_packet_unref(pPacket);
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

int PacketQueue::packetQueuePutPrivate(AVPacket *avPacket) {
    if (!avPacket) {
        return NEGATIVE(S_NULL);
    }
    if (abortRequest) {
        return NEGATIVE(S_NOT_ABORT_REQUEST);
    }
    auto *packetList = new MyAVPacketList();
    if (!packetList) {
        return NEGATIVE(S_NOT_MEMORY);
    }
    packetList->pkt = *avPacket;
    packetList->next = nullptr;
    if (avPacket == flushPacket) {
        serial++;
    }
    packetList->serial = serial;
    if (!lastPacketList) {
        firstPacketList = packetList;
    } else {
        lastPacketList->next = packetList;
    }
    lastPacketList = packetList;
    nbPackets++;
    // TODO
    //size += packetList->pkt.size + sizeof(*packetList);
    size++;
    duration += packetList->pkt.duration;
    /* XXX: should duplicate packet data in DV case */
    mutex->condSignal();
    return POSITIVE;
}


int PacketQueue::packetQueueDestroy() {
    packetQueueFlush();
    delete mutex;
    mutex = nullptr;
    return POSITIVE;
}

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

#pragma clang diagnostic pop