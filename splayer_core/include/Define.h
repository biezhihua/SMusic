#ifndef SPLAYER_CORE_DEFINE_H
#define SPLAYER_CORE_DEFINE_H

#define VERSION_MODULE_FILE_NAME_LENGTH     13

// Queue Size
#define VIDEO_QUEUE_SIZE                    3
#define AUDIO_QUEUE_SIZE                    9
#define SUBTITLE_QUEUE_SIZE                 16
#define FRAME_QUEUE_SIZE                    FFMAX(AUDIO_QUEUE_SIZE, FFMAX(VIDEO_QUEUE_SIZE, SUBTITLE_QUEUE_SIZE))
#define MAX_QUEUE_SIZE                      (15 * 1024 * 1024)

#define MIN_FRAMES                          25

#define EXTERNAL_CLOCK_MIN_FRAMES           2
#define EXTERNAL_CLOCK_MAX_FRAMES           10

/// 根据实时码流的缓冲区填充时间做外部时钟调整
/// 最小值
#define EXTERNAL_CLOCK_SPEED_MIN            0.900
/// 最大值
#define EXTERNAL_CLOCK_SPEED_MAX            1.010
/// 步进
#define EXTERNAL_CLOCK_SPEED_STEP           0.001

/// 音量 Volume
#define MIX_MAX_VOLUME                      128

/// 同步类型 Sync Type
/// 音频作为同步，默认以音频同步
#define SYNC_TYPE_AUDIO_MASTER              0 /* default choice */
/// 视频作为同步
#define SYNC_TYPE_VIDEO_MASTER              1
/// 外部时钟作为同步
#define SYNC_TYPE_EXTERNAL_CLOCK            2 /* synchronize to an external clock */

/// 显示类型 Show Mode
/// 无显示
#define SHOW_MODE_NONE                      -1
/// 显示视频
#define SHOW_MODE_VIDEO                     0
/// 显示波浪，音频
#define SHOW_MODE_WAVES                     1
/// 自适应滤波器
#define SHOW_MODE_RDFT                      2
#define SHOW_MODE_NB                        3

/// 最低同步阈值，如果低于该值，则不需要同步校正
/// no sync correction is done if below the minimum AV sync threshold
#define SYNC_THRESHOLD_MIN                  0.04

/// 最大同步阈值，如果大于该值，则需要同步校正
/// sync correction is done if above the maximum sync threshold
#define SYNC_THRESHOLD_MAX                  0.1

/// 帧补偿同步阈值，如果帧持续时间比这更长，则不用来补偿同步
/// If a frame duration is longer than this, it will not be duplicated to compensate AV sync
#define SYNC_FRAMEDUP_THRESHOLD             0.1

/// 同步阈值。如果误差太大，则不进行校正
/// no correction is done if too big error
#define NO_SYNC_THRESHOLD                   10.0

/// 对于可能需要的屏幕刷新视频的轮询，应该小于1/fps
/// polls for possible required screen refreshVideo at least this often, should be less than 1/fps
#define REFRESH_RATE                        0.01

/// 一秒
/// 1 second
#define CURSOR_HIDE_DELAY                   (1000000*1.0F)

/// 使用差值来实现平均值
/// we use about AUDIO_DIFF_AVG_NB A-V differences to make the average
#define AUDIO_DIFF_AVG_NB                   20

/// 最小音频缓冲
/// Minimum SDL audio buffer size, in samples.
#define SDL_AUDIO_MIN_BUFFER_SIZE           512

/// 计算实际音频缓冲大小，并不需要太频繁回调，这里设置的是最大音频回调次数是每秒30次
/// Calculate actual buffer size keeping in mind not cause too frequent audio callbacks
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC     30

/// Step size for volume control in dB
/// 音频控制 以db为单位的步进
#define SDL_VOLUME_STEP                     (0.75)

/// 正确同步的最大音频速度变化值(百分比)
/// maximum audio speed change to get correct sync
#define SAMPLE_CORRECTION_PERCENT_MAX       10

// 采样大小
#define SAMPLE_ARRAY_SIZE (8 * 65536)

#endif //SPLAYER_CORE_DEFINE_H
