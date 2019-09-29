#include <queue/PacketQueue.h>

PacketQueue::PacketQueue(AVPacket *flushPacket) {
    this->flushPacket = flushPacket;
    abortRequest = false;
    firstPacket = nullptr;
    lastPacket = nullptr;
    packetSize = 0;
    memorySize = 0;
    duration = 0;
}

PacketQueue::~PacketQueue() {
    abort();
    flush();
    this->flushPacket = nullptr;
}

/**
 * 入队数据包
 * @param pkt
 * @return
 */
int PacketQueue::put(AVPacket *pkt) {
    PacketData *packetData;

    if (abortRequest) {
        return ERROR_ABORT_REQUEST;
    }

    packetData = (PacketData *) av_malloc(sizeof(PacketData));
    if (!packetData) {
        return ERROR_NOT_MEMORY;
    }
    packetData->pkt = *pkt;
    packetData->next = nullptr;

    if (pkt == flushPacket) {
        lastSeekSerial++;
    }
    packetData->serial = lastSeekSerial;

    if (!lastPacket) {
        firstPacket = packetData;
    } else {
        lastPacket->next = packetData;
    }
    lastPacket = packetData;
    packetSize++;
    memorySize += packetData->pkt.size + sizeof(*packetData);
    duration += packetData->pkt.duration;
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

int PacketQueue::putNullPacket(int stream_index) {
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
    PacketData *pkt, *pkt1;
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
    put(flushPacket);
    condition.signal();
    mutex.unlock();
}

/**
 * 取出数据包
 * @param pkt
 * @return
 */
int PacketQueue::getPacket(AVPacket *pkt) { return getPacket(pkt, 1); }

/**
 * 取出数据包
 * @param pkt
 * @param block
 * @return
 */
int PacketQueue::getPacket(AVPacket *pkt, int block) {
    PacketData *pkt1;
    int ret;
    mutex.lock();
    for (;;) {
        if (abortRequest) {
            if (DEBUG) ALOGD(TAG, "%s abort request", __func__);
            ret = ERROR_ABORT_REQUEST;
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
            firstSeekSerial = pkt1->serial;
            av_free(pkt1);
            ret = SUCCESS;
            break;
        } else if (!block) {
            ret = SUCCESS;
            if (DEBUG) ALOGD(TAG, "%s not block", __func__);
            break;
        } else {
            if (DEBUG) ALOGD(TAG, "%s packet queue wait", __func__);
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
    Mutex::Autolock lock(mutex);
    return memorySize;
}

int64_t PacketQueue::getDuration() {
    Mutex::Autolock lock(mutex);
    return duration;
}

bool PacketQueue::isAbort() {
    Mutex::Autolock lock(mutex);
    return abortRequest;
}

int PacketQueue::getLastSeekSerial() {
    Mutex::Autolock lock(mutex);
    return lastSeekSerial;
}

int PacketQueue::getFirstSeekSerial() {
    Mutex::Autolock lock(mutex);
    return firstSeekSerial;
}

int *PacketQueue::getPointLastSeekSerial() {
    Mutex::Autolock lock(mutex);
    return &lastSeekSerial;
}

int PacketQueue::signal() {
    mutex.lock();
    condition.signal();
    mutex.unlock();
    return SUCCESS;
}
