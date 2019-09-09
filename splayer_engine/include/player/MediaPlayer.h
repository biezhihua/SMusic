#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <sync/MediaClock.h>
#include <player/PlayerState.h>
#include <decoder/AudioDecoder.h>
#include <decoder/VideoDecoder.h>
#include <device/AudioDevice.h>
#include <device/VideoDevice.h>
#include <sync/MediaSync.h>
#include <convertor/AudioResampler.h>
#include <common/Log.h>
#include <message/MessageDevice.h>

class MediaPlayer : public Runnable {

    const char *const TAG = "MediaPlayer";

    const char *const OPT_SCALL_ALL_PMTS = "scan_all_pmts";

    const char *const OPT_HEADERS = "headers";

    const char *const FORMAT_OGG = "ogg";

    const char *const OPT_LOW_RESOLUTION = "lowres";

    const char *const OPT_THREADS = "threads";

    const char *const OPT_REF_COUNTED_FRAMES = "refcounted_frames";

private:
    Mutex waitMutex;
    Condition waitCondition;

    Mutex mutex;
    Condition condition;

    Thread *readThread;                     // 读数据包线程
    Thread *msgThread;

    PlayerState *playerState;               // 播放器状态

    AudioDecoder *audioDecoder;             // 音频解码器
    VideoDecoder *videoDecoder;             // 视频解码器

    MessageDevice *msgDevice;               // 消息处理

    bool quit;                              // state for reading packets thread exited if not

    // 解复用处理
    AVFormatContext *formatContext;         // 解码上下文
    int64_t duration;                       // 文件总时长
    int lastPaused;                         // 上一次暂停状态
    int eof;                                // 数据包读到结尾标志
    int attachmentRequest;                  // 视频封面数据包请求

    VideoDevice *videoDevice;               // 视频输出设备

    AudioDevice *audioDevice;               // 音频输出设备
    AudioResampler *audioResampler;         // 音频重采样器

    MediaSync *mediaSync;                   // 媒体同步器

    AVPacket flushPacket;                   // 刷新的包,用于在SEEK时，刷新数据队列

public:
    MediaPlayer();

    virtual ~MediaPlayer();

    int reset();

    void setDataSource(const char *url, int64_t offset = 0, const char *headers = nullptr);

    int prepareAsync();

    void start();

    void pause();

    void resume();

    void stop();

    void seekTo(float timeMs);

    void setLooping(int looping);

    void setVolume(float leftVolume, float rightVolume);

    void setMute(int mute);

    void setRate(float rate);

    void setPitch(float pitch);

    int getRotate();

    int getVideoWidth();

    int getVideoHeight();

    long getCurrentPosition();

    long getDuration();

    int isPlaying();

    int isLooping();

    int getMetadata(AVDictionary **metadata);

    MessageQueue *getMessageQueue();

    PlayerState *getPlayerState();

    void pcmQueueCallback(uint8_t *stream, int len);

    void setAudioDevice(AudioDevice *audioDevice);

    void setMediaSync(MediaSync *mediaSync);

    MediaSync *getMediaSync() const;

    void setVideoDevice(VideoDevice *videoDevice);

    void setMessageDevice(MessageDevice *msgDevice);

protected:
    void run() override;

private:
    int readPackets();

    // prepareAsync decoder with stream_index
    int prepareDecoder(int streamIndex);

    // open an audio output device
    int openAudioDevice(int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate);

    bool isNoReadMore() const;

    bool isRetryPlay() const;

    bool isPacketInPlayRange(const AVFormatContext *formatContext, const AVPacket *packet) const;

    void togglePause();
};


#endif //MEDIAPLAYER_H
