#ifndef MEDIASYNC_H
#define MEDIASYNC_H

#include <sync/MediaClock.h>
#include <player/PlayerState.h>
#include <decoder/VideoDecoder.h>
#include <decoder/AudioDecoder.h>

#include <device/VideoDevice.h>

/**
 * 视频同步器
 */
class MediaSync : public Runnable {

    const char *const TAG = "MediaSync";

public:
    MediaSync();

    virtual ~MediaSync();

    void reset();

    void start(VideoDecoder *videoDecoder, AudioDecoder *audioDecoder);

    void stop();

    // 设置视频输出设备
    void setVideoDevice(VideoDevice *device);

    // 设置帧最大间隔
    void setMaxDuration(double maxDuration);

    // 更新音频时钟
    void updateAudioClock(double pts, double time);

    // 获取音频时钟与主时钟的差值
    double getAudioDiffClock();

    // 更新外部时钟
    void updateExternalClock(double pts, int seekSerial);

    double getMasterClock();

    void run() override;

    MediaClock *getAudioClock();

    MediaClock *getVideoClock();

    MediaClock *getExternalClock();

    void setPlayerState(PlayerState *playerState);

    void togglePause();

private:
    void refreshVideo(double *remaining_time);

    void checkExternalClockSpeed();

    double calculateDelay(double delay);

    double calculateDuration(Frame *current, Frame *next);

    void renderVideo();

protected:

    void resetRemainingTime();

    void refreshVideo();

protected:

    /// 播放器状态
    PlayerState *playerState = nullptr;

    /// 停止
    bool abortRequest;

    bool quit;

    /// 音频时钟
    MediaClock *audioClock = nullptr;

    /// 视频时钟
    MediaClock *videoClock = nullptr;

    /// 外部时钟
    MediaClock *externalClock = nullptr;

    /// 视频解码器
    VideoDecoder *videoDecoder = nullptr;

    /// 视频解码器
    AudioDecoder *audioDecoder = nullptr;

    Mutex mutex;

    Condition condition;

    /// 强制刷新标志
    int forceRefresh;

    /// 最大帧延时
    double maxFrameDuration;

    /// 视频时钟
    double frameTimer;

    /// 视频输出设备
    VideoDevice *videoDevice = nullptr;

    AVFrame *frameARGB = nullptr;
    uint8_t *buffer = nullptr;
    SwsContext *swsContext = nullptr;

    /// 帧间隔时间
    double remainingTime = 0.0;
};


#endif //MEDIASYNC_H
