#ifndef SPLAYER_MAC_OPTIONS_H
#define SPLAYER_MAC_OPTIONS_H

#include "Define.h"
#include "Log.h"
#include "Mutex.h"

extern "C" {
#include <libavutil/dict.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <config.h>
};

#define OPTIONS_TAG "Options"

class Options {

private:

    void showDict(const char *tag, AVDictionary *dict);

public:

    Options();

    virtual ~Options();

    /**
    * Format/Codec Options
    */
    AVDictionary *format = nullptr;
    AVDictionary *codec = nullptr;
    AVDictionary *swsDict = nullptr;
    AVDictionary *swrDict = nullptr;
    AVDictionary *swrPresetDict = nullptr;
    AVDictionary *playerDict = nullptr;

    /**
     * Options specified by the user
     */

    /// force format name
    char *inputFormatName = nullptr;

    /// force format
    AVInputFormat *inputFormat = nullptr;

    /// 文件名称
    char *inputFileName = nullptr;

    /// 视频名称
    char *videoTitle = nullptr;

    /// 视频宽度
    int videoWidth = 640;

    /// 视频高度
    int videoHeight = 480;

    /// 画布宽度
    int surfaceWidth = 0;

    /// 画布高度
    int surfaceHeight = 0;

    /// 画布X轴偏移位置
    int surfaceLeftOffset = 0;

    /// 画布Y轴偏移位置
    int surfaceTopOffset = 0;

    /// 预设流
    /// "select desired stream"
    char *wantedStreamSpec[AVMEDIA_TYPE_NB] = {nullptr};

    /// 是否禁止视频
    int videoDisable;

    /// 是否禁止音频
    int audioDisable = 1;

    /// 是否禁止字幕
    int subtitleDisable = 1;

    /// 按字节快进或者快退 0=off 1=on -1=auto
    /// seek by bytes 0=off 1=on -1=auto
    int seekByBytes = -1;

    /// 快进或者快推的间隔(秒)
    /// set seek interval for left/right keys, in seconds
    float seekInterval = 10;

    /// 启动音量
    int audioStartupVolume = 100; // set startup volume 0=min 100=max

    /// 音视频同步类型
    /// 1. 按音频为主进行同步
    /// 2. 按视频为主进行同步
    /// set audio-video sync. type (type=audio/video/ext)
    int syncType = SYNC_TYPE_AUDIO_MASTER;

    /// 流启动位置时间（秒）
    /// seek to a given position in seconds
    int64_t startTime = AV_NOPTS_VALUE;

    /// 流时长
    /// stream  \"duration\" seconds of audio/video
    int64_t duration = AV_NOPTS_VALUE;

    /// 是否允许非规范兼容的流加速
    /// non spec compliant optimizations
    int fast = 0;

    /// 是否生成丢失的PTS
    /// generate missing pts
    int generateMissingPts = 0;

    /// 是否启动低分辨率解码  1 -> 1/2 size, 2 -> 1/4 size
    /// low resolution decoding, 1-> 1/2 size, 2->1/4 size
    int lowResolution = 0;

    /// 是否自动旋转视频
    /// automatically rotate video
    int autoRotate = 1;

    /// 是否使用解码器估算过的时间来矫正PTS 0=off 1=on -1=auto
    /// let decoder reorder pts 0=off 1=on -1=auto
    int decoderReorderPts = -1;

    /// 结束播放时自动退出
    /// exit at the end
    int autoExit = 0;

    /// 设置循环播放的次数
    /// set number of times the playback shall be looped
    int loopTimes = 1;

    /// 是否丢帧，当CPU过慢时
    /// drop frames when cpu is too slow
    int dropFrameWhenSlow = -1;

    /// 画面显示模式 SHOW_MODE_VIDEO / SHOW_MODE_WAVES / SHOW_MODE_RDFT
    /// select show mode (0 = video, 1 = waves, 2 = RDFT)
    int showMode = SHOW_MODE_NONE;

    /// 音频解码器名称，可能会导致失败
    char *forceAudioCodecName = nullptr;

    /// 字幕解码器名称，可能会导致失败
    char *forceSubtitleCodecName = nullptr;

    /// 视频解码器名称，可能会导致失败
    char *forceVideoCodecName = nullptr;

    /// 不设置缓存区限制，通常对实时流很有用
    /// don't limit the input buffer size (useful with realtime streams)
    int infiniteBuffer = -1;

    /// 是否显示FFmpeg状态信息
    int showStatus = 1;

    /// 智能填充缺失的信息
    /// read and decode the streams to fill missing information with heuristics
    int findStreamInfoFillMissingInformation = 1;

    ///
    double rdftSpeed = 0.02;  // rdft speed

#if CONFIG_AVFILTER

    ///
    int filterNumberThreads = 0;

    ///
    const char **filtersList = nullptr;

    ///
    int nb_vfilters = 0;

    ///
    char *afilters = nullptr;
#endif

    virtual void showOptions();

};

#endif //SPLAYER_MAC_OPTIONS_H
