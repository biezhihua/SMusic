#ifndef ENGINE_STREAM_H
#define ENGINE_STREAM_H

class MediaPlayer;

#include <MediaPlayer.h>
#include <common/Thread.h>
#include <player/PlayerInfoStatus.h>
#include <decoder/AudioDecoder.h>
#include <decoder/VideoDecoder.h>
#include <sync/MediaSync.h>
#include <stream/IStreamListener.h>

class Stream : public Runnable {

    const char *const TAG = "[MP][NATIVE][Stream]";

    const char *const OPT_SCALL_ALL_PMTS = "scan_all_pmts";

    const char *const OPT_HEADERS = "headers";

    const char *const FORMAT_OGG = "ogg";

    const char *const FORMAT_RTMP = "rtmp";

    const char *const FORMAT_RTSP = "rtsp";

    const char *const OPT_KEY_TIMEOUT = "timeout";

private:
    /// 读数据包线程
    Thread *readThread = nullptr;

    /// 播放器
    MediaPlayer *mediaPlayer = nullptr;

    /// 播放器状态
    PlayerInfoStatus *playerState = nullptr;

    /// 音频解码器
    AudioDecoder *audioDecoder = nullptr;

    /// 视频解码器
    VideoDecoder *videoDecoder = nullptr;

    /// 媒体同步器
    MediaSync *mediaSync = nullptr;

    MessageCenter *messageCenter = nullptr;

    /// 解码上下文
    AVFormatContext *formatContext = nullptr;

    /// 刷新的包,用于在SEEK时，刷新数据队列
    AVPacket flushPacket;

    IStreamListener *streamListener = nullptr;

    Mutex waitMutex;

    Condition waitCondition;

    Mutex mutex;

    Condition condition;

public:

    Stream(MediaPlayer *mediaPlayer, PlayerInfoStatus *playerState);

    ~Stream() override;

    void run() override;

    int create();

    int start();

    int stop();

    int destroy();

    void setAudioDecoder(AudioDecoder *audioDecoder);

    void setVideoDecoder(VideoDecoder *videoDecoder);

    void setMediaSync(MediaSync *mediaSync);

    Condition *getWaitCondition();

    AVPacket *getFlushPacket();

    void setStreamListener(IStreamListener *streamListener);

    void setMessageCenter(MessageCenter *messageCenter);

private:

    int readPackets();

    bool isFinish() const;

    bool isPacketInPlayRange(const AVFormatContext *formatContext, const AVPacket *packet) const;

    int openStream();

    int notifyMsg(int what);

    int notifyMsg(int what, int arg1);

    int notifyMsg(int what, int arg1, int arg2);

    void doSeek() ;

    void doPause() const;

    int doAttachment() const;

    void doRetryPlay();

    bool isNotReadMore() const;
};


#endif
