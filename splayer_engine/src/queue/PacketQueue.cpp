#include "queue/PacketQueue.h"

PacketQueue::PacketQueue() {
    abortRequest = 0;
    firstPacket = nullptr;
    lastPacket = nullptr;
    packetSize = 0;
    memorySize = 0;
    duration = 0;
}

PacketQueue::~PacketQueue() {
    abort();
    flush();
}

/**
 * 入队数据包
 * @param pkt
 * @return
 */
int PacketQueue::put(AVPacket *pkt) {
    PacketList *pkt1;

    if (abortRequest) {
        return -1;
    }

    pkt1 = (PacketList *) av_malloc(sizeof(PacketList));
    if (!pkt1) {
        return -1;
    }
    pkt1->pkt = *pkt;
    pkt1->next = nullptr;

    if (!lastPacket) {
        firstPacket = pkt1;
    } else {
        lastPacket->next = pkt1;
    }
    lastPacket = pkt1;
    packetSize++;
    memorySize += pkt1->pkt.size + sizeof(*pkt1);
    duration += pkt1->pkt.duration;
    return SUCCESS;
}

/**
 * 入队数据包
 * @param pkt
 * @return
 */
int PacketQueue::pushPacket(AVPacket *pkt) {
    int ret;
    mutex.lock();
    ret = put(pkt);
    condition.signal();
    mutex.unlock();

    if (ret < 0) {
        av_packet_unref(pkt);
    }

    return ret;
}

int PacketQueue::pushNullPacket(int stream_index) {
    AVPacket pkt1, *pkt = &pkt1;
    av_init_packet(pkt);
    pkt->data = nullptr;
    pkt->size = 0;
    pkt->stream_index = stream_index;
    return pushPacket(pkt);
}

/**
 * 刷新数据包
 */
void PacketQueue::flush() {
    PacketList *pkt, *pkt1;
    mutex.lock();
    for (pkt = firstPacket; pkt; pkt = pkt1) {
        pkt1 = pkt->next;
        av_packet_unref(&pkt->pkt);
        av_freep(&pkt);
    }
    lastPacket = nullptr;
    firstPacket = nullptr;
    packetSize = 0;
    memorySize = 0;
    duration = 0;
    condition.signal();
    mutex.unlock();
}

/**
 * 队列终止
 */
void PacketQueue::abort() {
    mutex.lock();
    abortRequest = true;
    condition.signal();
    mutex.unlock();
}

/**
 * 队列开始
 */
void PacketQueue::start() {
    mutex.lock();
    abortRequest = false;
    condition.signal();
    mutex.unlock();
}

/**
 * 取出数据包
 * @param pkt
 * @return
 */
int PacketQueue::getPacket(AVPacket *pkt) {
    return getPacket(pkt, 1);
}

/**
 * 取出数据包
 * @param pkt
 * @param block
 * @return
 */
int PacketQueue::getPacket(AVPacket *pkt, int block) {
    PacketList *pkt1;
    int ret;

    mutex.lock();
    for (;;) {
        if (abortRequest) {
            ret = -1;
            break;
        }

        pkt1 = firstPacket;
        if (pkt1) {
            firstPacket = pkt1->next;
            if (!firstPacket) {
                lastPacket = nullptr;
            }
            packetSize--;
            memorySize -= pkt1->pkt.size + sizeof(*pkt1);
            duration -= pkt1->pkt.duration;
            *pkt = pkt1->pkt;
            av_free(pkt1);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            condition.wait(mutex);
        }
    }
    mutex.unlock();
    return ret;
}

int PacketQueue::getPacketSize() {
    Mutex::Autolock lock(mutex);
    return packetSize;
}

int PacketQueue::getSize() {
    return memorySize;
}

int64_t PacketQueue::getDuration() {
    return duration;
}

int PacketQueue::isAbort() {
    return abortRequest;
}
