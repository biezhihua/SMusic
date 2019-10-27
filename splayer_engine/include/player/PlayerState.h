#ifndef  ENGINE_PLAYER_STATE_H
#define  ENGINE_PLAYER_STATE_H

#include <common/Mutex.h>
#include <common/Condition.h>
#include <common/Thread.h>
#include <common/FFmpegUtils.h>
#include <common/Log.h>
#include <message/MessageQueue.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mem.h>
#include <libavutil/rational.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/avstring.h>
};


#define VIDEO_QUEUE_SIZE                            3
#define AUDIO_QUEUE_SIZE                           9

#define MAX_QUEUE_SIZE                              (15 * 1024 * 1024)
#define MIN_FRAMES                                  25
#define FRAME_QUEUE_SIZE                            10

/// 最小音频缓冲
/// Minimum SDL audio buffer size, in samples.
#define AUDIO_MIN_BUFFER_SIZE                       512

/// 计算实际音频缓冲大小，并不需要太频繁回调，
/// 这里设置的是最大音频回调次数是每秒30次
/// Calculate actual buffer size keeping in mind
/// not cause too frequent audio callbacks
#define AUDIO_MAX_CALLBACKS_PER_SEC                 30

/// 对于可能需要的屏幕刷新视频的轮询，应该小于1/fps
/// polls for possible required screen refreshVideo at least this often,
/// should be less than 1/fps
#define REFRESH_RATE                                0.01

/// 最低同步阈值，如果低于该值，则不需要同步校正
/// no sync correction is done if below the minimum AV sync threshold
#define AV_SYNC_THRESHOLD_MIN                       0.04

/// 最大同步阈值，如果大于该值，则需要同步校正
/// sync correction is done if above the maximum sync threshold
#define AV_SYNC_THRESHOLD_MAX                       0.1

/// 帧补偿同步阈值，如果帧持续时间比这更长，则不用来补偿同步
/// If a frame duration is longer than this,
/// it will not be duplicated to compensate AV sync
#define AV_SYNC_FRAMEDUP_THRESHOLD                  0.1

/// 同步阈值。如果误差太大，则不进行校正
/// no correction is done if too big error
#define AV_NOSYNC_THRESHOLD                         10.0

#define EXTERNAL_CLOCK_MIN_FRAMES                   2

#define EXTERNAL_CLOCK_MAX_FRAMES                   10

/// 根据实时码流的缓冲区填充时间做外部时钟调整 最小值
#define EXTERNAL_CLOCK_SPEED_MIN                    0.900

/// 根据实时码流的缓冲区填充时间做外部时钟调整 最大值
#define EXTERNAL_CLOCK_SPEED_MAX                    1.010

/// 根据实时码流的缓冲区填充时间做外部时钟调整 步进
#define EXTERNAL_CLOCK_SPEED_STEP                   0.001

/// 使用差值来实现平均值
/// we use about AUDIO_DIFF_AVG_NB A-V differences to make the average
#define AUDIO_DIFF_AVG_NB                           20

/// 正确同步的最大音频速度变化值(百分比)
/// maximum audio speed change to get correct sync
#define SAMPLE_CORRECTION_PERCENT_MAX               10

/// Options 定义
#define OPT_CATEGORY_FORMAT                         1
#define OPT_CATEGORY_CODEC                          2
#define OPT_CATEGORY_SWS                            3
#define OPT_CATEGORY_PLAYER                         4
#define OPT_CATEGORY_SWR                            5

typedef enum {
    AV_SYNC_AUDIO,      // 同步到音频时钟
    AV_SYNC_VIDEO,      // 同步到视频时钟
    AV_SYNC_EXTERNAL,   // 同步到外部时钟
} SyncType;

struct AVDictionary {
    int count;
    AVDictionaryEntry *elements;
};


/**
 * MediaPlayer Status 播放器状态
 *
 * IDLED            -> 已闲置态
 * CREATED          -> 已创建态
 * STARTED          -> 已启动态
 * PLAYING          -> 已播放态
 * PAUSED           -> 已暂停态
 * STOPED           -> 已停止态
 * DESTROYED        -> 已销毁态
 * ERRORED          -> 已错误态
 */
enum MediaPlayerState {
    /**
     * new MediaPlayer()        => self
     * destroy()                => self
     * reset()                  => self
     * create()                 => CREATED
     */
            IDLED = 1,
    /**
     * create()                 => self
     * start()                  => STARTED
     * destroy()                => DESTROYED -> IDLE
     * reset()                  => DESTROYED -> IDLE
     */
            CREATED = 2,
    /**
     * start()                  => self
     * play()                   => self
     * pause()                  => PAUSED
     * stop()                   => STOPED
     * destroy()                => STOPED -> DESTROYED -> IDLE
     * reset()                  => STOPED -> DESTROYED -> IDLE
     */
            STARTED = 3,
    /**
     * STARTED & read a frame   => self
     * seek()                   => self
     * pause()                  => PAUSED
     * stop()                   => STOPED
     * destroy()                => STOPED -> DESTROYED -> IDLE
     * reset()                  => STOPED -> DESTROYED -> IDLE
     */
            PLAYING = 4,
    /**
     * seek()                   => self
     * paused()                 => self
     * play()                   => STARTED
     * stop()                   => STOPED
     * destroy()                => STOPED -> DESTROYED -> IDLE
     * reset()                  => STOPED -> DESTROYED -> IDLE
     */
            PAUSED = 5,
    /**
     * stop()                   => self
     * start()                  => STARTED
     * destroy()                => DESTROYED -> IDLE
     * reset()                  => DESTROYED -> IDLE
     */
            STOPED = 6,
    /**
     * destroy()                => self -> IDLE
     * reset()                  => self -> IDLE
     */
            DESTROYED = 7,
    /**
     *  any error               => ERRORED
     *  reset()                 => DESTROYED -> IDLE
     *  destroy()               => any -> DESTROY -> IDLE
     */
            ERRORED = 8
};

class PlayerState {

    const char *const TAG = "PlayerState";

public:
    PlayerState();

    virtual ~PlayerState();

    void reset();

    void setOption(int category, const char *type, const char *option);

    void setOptionLong(int category, const char *type, int64_t option);

private:

    void init();

    void parse_string(const char *type, const char *option);

public:
    void setFormatContext(AVFormatContext *formatContext);

private:
    void parse_int(const char *type, int64_t option);

public:

    /// 操作互斥锁，主要是给seek操作、音视频解码以及清空解码上下文缓冲使用，
    /// 不加锁会导致ffmpeg内部崩溃现象
    Mutex mutex;

    /// 视频转码option参数
    AVDictionary *swsOpts = nullptr;

    /// 音频重采样option参数
    AVDictionary *swrOpts = nullptr;

    /// 解复用option参数
    AVDictionary *formatOpts = nullptr;

    /// 解码option参数
    AVDictionary *codecOpts = nullptr;

    /// 指定文件封装格式，也就是解复用器
    AVInputFormat *inputFormat = nullptr;

    /// 文件路径
    const char *url = nullptr;

    /// 文件偏移量
    int64_t offset;

    /// 文件头信息
    const char *headers = nullptr;

    /// 视频名称
    const char *videoTitle = nullptr;

    /// 指定音频解码器名称
    char *audioCodecName = nullptr;

    /// 指定视频解码器名称
    char *videoCodecName = nullptr;

    /// 退出标志
    volatile int abortRequest;

    /// 暂停标志
    volatile int pauseRequest;

    /// 是否处于播放中
    volatile bool isPlaying;

    /// 上一次暂停状态
    int lastPaused;

    /// 同步类型
    SyncType syncType;

    /// 播放起始位置
    int64_t startTime;

    /// 流时长
    int64_t duration;

    /// 流时长 秒
    int64_t durationSec;

    /// 判断是否实时流
    int realTime;

    /// 是否无限缓冲区，默认为-1
    int infiniteBuffer;

    /// 是否禁止音频流
    int audioDisable;

    /// 是否禁止视频流
    int videoDisable;

    /// 是否禁止显示
    int displayDisable;

    /// 是否允许非规范兼容的流加速
    /// 解码上下文的AV_CODEC_FLAG2_FAST标志
    int fast;

    /// 是否生成丢失的PTS
    /// generate missing pts
    int generateMissingPts;

    /// 是否启动低分辨率解码  1 -> 1/2 size, 2 -> 1/4 size
    /// low resolution decoding, 1-> 1/2 size, 2->1/4 size
    int lowResolution;

    /// 播放速度
    float playbackRate;

    /// 播放音调
    float playbackPitch;

    /// 快进或者快推的间隔(秒)
    /// set seek interval for left/right keys, in seconds
    float seekInterval = 10;

    /// 是否以字节定位
    volatile int seekByBytes;

    /// 定位请求
    volatile int seekRequest;

    /// 定位标志
    int seekFlags;

    /// 定位位置
    int64_t seekPos;

    /// 定位偏移
    int64_t seekRel;

    /// 结束播放时自动退出
    /// exit at the end
    int autoExit;

    /// 设置循环播放的次数
    /// set number of times the playback shall be looped
    int loopTimes;

    /// 静音播放
    int audioMute;

    /// 是否丢帧，当CPU过慢时
    /// drop frames when cpu is too slow
    int dropFrameWhenSlow;

    /// 解码器重新排列时间戳
    /// 是否使用解码器估算过的时间来矫正PTS 0=off 1=on -1=auto
    /// let decoder reorder pts 0=off 1=on -1=auto
    int decoderReorderPts;

    /// 暂停基于网络的流状态
    int readPauseReturn;

    /// 数据包读到结尾标志
    int eof;

    /// 视频封面数据包请求
    int attachmentRequest;

    /// 解码上下文
    AVFormatContext *formatContext = nullptr;

    /// 视频流索引
    int videoIndex;

    /// 音频流索引
    int audioIndex;

    const char *getSyncType();

    void setAbortRequest(int abortRequest);

    void setPauseRequest(int pauseRequest);

    void setSeekRequest(int seekRequest);

};


#endif
