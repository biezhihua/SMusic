#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

class Stream;

#include <stream/Stream.h>
#include <sync/MediaClock.h>
#include <player/PlayerState.h>
#include <decoder/AudioDecoder.h>
#include <decoder/VideoDecoder.h>
#include <device/AudioDevice.h>
#include <device/VideoDevice.h>
#include <sync/MediaSync.h>
#include <convertor/AudioResample.h>
#include <common/Log.h>
#include <message/MessageCenter.h>
#include <stream/IStreamListener.h>
#include "IMediaPlayer.h"

class MediaPlayer : public IMediaPlayer, IStreamListener {

    const char *const TAG = "MediaPlayer";

    const char *const OPT_LOW_RESOLUTION = "lowResolution";

    const char *const OPT_THREADS = "threads";

    const char *const OPT_REF_COUNTED_FRAMES = "refcounted_frames";

protected:

    Mutex mutex;

    Condition condition;

    /// 播放器状态
    PlayerState *playerState = nullptr;

    /// 媒体同步器
    MediaSync *mediaSync = nullptr;

    /// 媒体流
    Stream *mediaStream = nullptr;

    /// 音频解码器
    AudioDecoder *audioDecoder = nullptr;

    /// 视频解码器
    VideoDecoder *videoDecoder = nullptr;

    /// 视频输出设备
    VideoDevice *videoDevice = nullptr;

    /// 音频输出设备
    AudioDevice *audioDevice = nullptr;

    /// 音频重采样器
    AudioResample *audioResample = nullptr;

    /// 消息事件处理
    MessageCenter *messageCenter = nullptr;

    /// 解码上下文
    AVFormatContext *formatContext = nullptr;

    /// 消息监听回调
    IMessageListener *messageListener = nullptr;

public:
    MediaPlayer();

    virtual ~MediaPlayer();

    int create() override;

    int start() override;

    int play() override;

    int pause() override;

    int stop() override;

    int destroy() override;

    int setDataSource(const char *url, int64_t offset = 0, const char *headers = nullptr);

    int seekTo(float timeMs);

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

    bool isPlaying();

    int isLooping();

    int getMetadata(AVDictionary **metadata);

    void pcmQueueCallback(uint8_t *stream, int len);

    void setAudioDevice(AudioDevice *audioDevice);

    void setMediaSync(MediaSync *mediaSync);

    void setVideoDevice(VideoDevice *videoDevice);

    int onStartOpenStream() override;

    int onEndOpenStream(int videoIndex, int audioIndex) override;

    void setMessageListener(IMessageListener *messageListener);

    void setFormatContext(AVFormatContext *formatContext);

protected:

    int _seek(float increment);

    int _seek(int64_t pos, int64_t rel, int seekByBytes);

    int _togglePause();

    int _stop();

    int _destroy();

    int _create();

    int _start();

    int _setDataSource(const char *url, int64_t offset, const char *headers) const;

    int openDecoder(int streamIndex);

    int openAudioDevice(int64_t wantedChannelLayout, int wantedNbChannels, int wantedSampleRate);

    int checkParams();

    int notifyMsg(int what);

    int notifyMsg(int what, int arg1);

    int notifyMsg(int what, int arg1, int arg2);
};


#endif //MEDIAPLAYER_H
