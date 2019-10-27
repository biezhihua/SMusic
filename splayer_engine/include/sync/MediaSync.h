#ifndef ENGINE_MEDIASYNC_H
#define ENGINE_MEDIASYNC_H

#include <sync/MediaClock.h>
#include <player/PlayerInfoStatus.h>
#include <decoder/VideoDecoder.h>
#include <decoder/AudioDecoder.h>
#include <device/VideoDevice.h>
#include <message/MessageCenter.h>

/**
 * 视频同步器
 */
class MediaSync : public Runnable {

    const char *const TAG = "MediaSync";

public:
    MediaSync();

    virtual ~MediaSync();

    int create();

    int destroy();

    virtual void start(VideoDecoder *videoDecoder, AudioDecoder *audioDecoder);

    virtual void stop();

    // 设置视频输出设备
    void setVideoDevice(VideoDevice *device);

    // 设置帧最大间隔
    void setMaxDuration(double maxDuration);

    // 更新音频时钟
    void updateAudioClock(double pts, int serial, double time);

    // 获取音频时钟与主时钟的差值
    double getAudioDiffClock();

    // 更新外部时钟
    void updateExternalClock(double pts, int seekSerial);

    double getMasterClock();

    void run() override;

    MediaClock *getAudioClock();

    MediaClock *getVideoClock();

    MediaClock *getExternalClock();

    void setPlayerState(PlayerInfoStatus *playerState);

    int togglePause();

    int notifyMsg(int what);

    int notifyMsg(int what, int arg1);

    int notifyMsg(int what, int arg1, int arg2);

    void resetRemainingTime();

    int refreshVideo();

    void setForceRefresh(int forceRefresh);

    void setMessageCenter(MessageCenter *messageCenter);

    void setMutex(Mutex *mMutex);

    void setCondition(Condition *mCondition);

private:

    int refreshVideo(double *remaining_time);

    void checkExternalClockSpeed();

    double calculateDelay(double delay);

    double calculateDuration(Frame *current, Frame *next);

    void renderVideo();

protected:

    Mutex mutex;

    Condition condition;

    Mutex *playerMutex;

    Condition *playerCondition;

    /// 停止
    bool abortRequest;

    /// 播放器状态
    PlayerInfoStatus *playerState = nullptr;

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

    ///
    MessageCenter *messageCenter = nullptr;

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


#endif
