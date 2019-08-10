#include "PacketQueue.h"

int PacketQueue::packetQueueInit(AVPacket *pFlushPacket) {

    flushPacket = pFlushPacket;

    mutex = new Mutex();

    if (!mutex) {
        ALOGE(PACKET_QUEUE_TAG, "%s create mutex fail", __func__);
        return NEGATIVE(S_NOT_MEMORY);
    }

    abortRequest = 1;

    ALOGD(PACKET_QUEUE_TAG, "%s mutex = %p abortRequest = %d",
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
        return NEGATIVE(S_ABORT_REQUEST);
    }
    auto *packetList = new PacketData();
    if (!packetList) {
        return NEGATIVE(S_NOT_MEMORY);
    }
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
    /* XXX: should duplicate packet data in DV case */
    mutex->condSignal();
    return POSITIVE;
}

