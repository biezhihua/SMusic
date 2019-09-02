#ifndef MEDIADECODER_H
#define MEDIADECODER_H

#include <Log.h>
#include <player/PlayerState.h>
#include <queue/PacketQueue.h>
#include <queue/FrameQueue.h>

class MediaDecoder : public Runnable {
public:
    MediaDecoder(AVCodecContext *codecContext, AVStream *stream, int streamIndex, PlayerState *playerState);

    virtual ~MediaDecoder();

    virtual void start();

    virtual void stop();

    virtual void flush();

    int pushPacket(AVPacket *pkt);

    int getPacketSize();

    int getStreamIndex();

    AVStream *getStream();

    AVCodecContext *getCodecContext();

    int getMemorySize();

    int hasEnoughPackets();

    virtual void run();

protected:
    Mutex mutex;
    Condition condition;
    bool abortRequest;
    PlayerState *playerState;
    PacketQueue *packetQueue;       // 数据包队列
    AVCodecContext *codecContext;
    AVStream *stream;
    int streamIndex;
};


#endif //MEDIADECODER_H
