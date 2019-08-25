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

    /// force format
    AVInputFormat *inputFormat = nullptr;

    /// 文件名称
    char *inputFileName = nullptr;

    /// 窗口名称
    char *windowTitle = nullptr;

    /// 视频宽度
    int videoWidth = 640;

    /// 视频高度
    int videoHeight = 480;

    /// 窗口宽度
    int screenWidth = 0;

    /// 窗口高度
    int screenHeight = 0;

    /// 窗口X轴偏移位置
    int screenLeft = 0;

    /// 窗口Y轴偏移位置
    int screenTop = 0;

    char *wantedStreamSpec[AVMEDIA_TYPE_NB] = {nullptr}; // "select desired stream"

    int videoDisable;
    int audioDisable = 1;
    int subtitleDisable = 1;

    int seekByBytes = -1; // seek by bytes 0=off 1=on -1=auto
    float seekInterval = 10; // set seek interval for left/right keys, in seconds

    int startupVolume = 100; // set startup volume 0=min 100=max
    int syncType = SYNC_TYPE_AUDIO_MASTER; // set audio-video sync. type (type=audio/video/ext)
    int64_t startTime = AV_NOPTS_VALUE; // seek to a given position in seconds
    int64_t duration = AV_NOPTS_VALUE; // stream  \"duration\" seconds of audio/video

    int fast = 0; // non spec compliant optimizations
    int generateMissingPts = 0; // generate missing pts
    int lowResolution = 0;
    int autoRotate = 1; // automatically rotate video
    int decoderReorderPts = -1; // let decoder reorder pts 0=off 1=on -1=auto
    int autoExit = 0; // exit at the end
    int loopTimes = 1; // set number of times the playback shall be looped
    int dropFrameWhenSlow = -1; // drop frames when cpu is too slow
    int showMode = SHOW_MODE_NONE; // select show mode (0 = video, 1 = waves, 2 = RDFT)
    char *inputFormatName = nullptr;
    char *forceAudioCodecName = nullptr; // force decoder
    char *forceSubtitleCodecName = nullptr; // force decoder
    char *forceVideoCodecName = nullptr; // force decoder
    double rdftSpeed = 0.02;  // rdft speed
    int infiniteBuffer = -1; // don't limit the input buffer size (useful with realtime streams)
    int showStatus = 1; // show status
    int findStreamInfoFillMissingInformation = 1; // read and decode the streams to fill missing information with heuristics
    int filterNBThreads = 0;
#if CONFIG_AVFILTER
    const char **vfiltersList = nullptr;
    int nb_vfilters = 0;
    char *afilters = nullptr;
#endif
    int64_t audio_callback_time;

    virtual void showOptions();

};

#endif //SPLAYER_MAC_OPTIONS_H
