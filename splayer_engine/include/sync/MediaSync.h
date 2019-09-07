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
    PlayerState *playerState;               // 播放器状态
    bool abortRequest;                      // 停止
    bool quit;

    MediaClock *audioClock;                 // 音频时钟
    MediaClock *videoClock;                 // 视频时钟
    MediaClock *externalClock;              // 外部时钟

    VideoDecoder *videoDecoder;             // 视频解码器
    AudioDecoder *audioDecoder;             // 视频解码器

    Mutex mutex;
    Condition condition;

    int forceRefresh;                       // 强制刷新标志
    double maxFrameDuration;                // 最大帧延时
    double frameTimer;                      // 视频时钟

    VideoDevice *videoDevice;               // 视频输出设备

    AVFrame *frameARGB;
    uint8_t *buffer;
    SwsContext *swsContext;

    double remainingTime = 0.0;            // 帧间隔时间
};


#endif //MEDIASYNC_H
