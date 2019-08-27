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

/* external clock speed adjustment constants for realtime sources based on buffer fullness */
#define EXTERNAL_CLOCK_SPEED_MIN            0.900
#define EXTERNAL_CLOCK_SPEED_MAX            1.010
#define EXTERNAL_CLOCK_SPEED_STEP           0.001

/// 音量 Volume
#define MIX_MAX_VOLUME                      128

/// 同步类型 Sync Type
#define SYNC_TYPE_AUDIO_MASTER              0 /* default choice */
#define SYNC_TYPE_VIDEO_MASTER              1
#define SYNC_TYPE_EXTERNAL_CLOCK            2 /* synchronize to an external clock */

/// 显示模式 Show Mode
#define SHOW_MODE_NONE                      -1
#define SHOW_MODE_VIDEO                     0
#define SHOW_MODE_WAVES                     1
#define SHOW_MODE_RDFT                      2
#define SHOW_MODE_NB                        3

/// 如果低于最小同步阈值，则不进行同步校正
/// no sync correction is done if below the minimum AV sync threshold
#define SYNC_THRESHOLD_MIN                  0.04

/// 如果超过最大同步阈值，则进行同步校正
/// sync correction is done if above the maximum sync threshold
#define SYNC_THRESHOLD_MAX                  0.1

/// 如果帧的持续时间比这个长，它将不会被复制来补偿AV同步
/// If a frame duration is longer than this, it will not be duplicated to compensate AV sync
#define SYNC_FRAMEDUP_THRESHOLD             0.1


/// 如果错误太大，则不作纠正
/// no correction is done if too big error
#define NO_SYNC_THRESHOLD                   10.0

/// 对于可能需要的屏幕刷新视频的轮询，应该小于1/fps
/// polls for possible required screen refreshVideo at least this often, should be less than 1/fps
#define REFRESH_RATE                        0.01

/// 一秒
/// 1 second
#define CURSOR_HIDE_DELAY                   (1000000*1.0F)

/// we use about AUDIO_DIFF_AVG_NB A-V differences to make the average
#define AUDIO_DIFF_AVG_NB                   20

/// Minimum SDL audio buffer size, in samples.
#define SDL_AUDIO_MIN_BUFFER_SIZE           512

/// Calculate actual buffer size keeping in mind not cause too frequent audio callbacks
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC     30

/// Step size for volume control in dB
#define SDL_VOLUME_STEP                     (0.75)

/// maximum audio speed change to get correct sync
#define SAMPLE_CORRECTION_PERCENT_MAX       10

#endif //SPLAYER_CORE_DEFINE_H
