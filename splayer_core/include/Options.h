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
    AVInputFormat *inputFormat = nullptr; // force format
    char *inputFileName = nullptr;
    char *windowTitle = nullptr; // set window title
    int isFullScreen = 0; // force full screen
    int defaultWidth = 640;
    int defaultHeight = 480;
    int screenWidth = 0; // force displayed width
    int screenHeight = 0; // force displayed height
    int screenLeft = 0; // set the x position for the left of the window
    int screenTop = 0;  // set the y position for the top of the window
    char *wantedStreamSpec[AVMEDIA_TYPE_NB] = {nullptr}; // "select desired stream"
    int seekByBytes = -1; // seek by bytes 0=off 1=on -1=auto
    float seekInterval = 10; // set seek interval for left/right keys, in seconds
    int startupVolume = 100; // set startup volume 0=min 100=max
    int syncType = SYNC_TYPE_AUDIO_MASTER; // set audio-video sync. type (type=audio/video/ext)
    int64_t startTime = AV_NOPTS_VALUE; // seek to a given position in seconds
    int64_t duration = AV_NOPTS_VALUE; // play  \"duration\" seconds of audio/video
    int borderLess = 0; // borderLess window
    int fast = 0; // non spec compliant optimizations
    int generatePts = 0; // generate pts
    int lowres = 0;
    int autoRotate = 0; // automatically rotate video
    int decoderReorderPts = -1; // let decoder reorder pts 0=off 1=on -1=auto
    int autoExit = 0; // exit at the end
    int loop = 1; // set number of times the playback shall be looped
    int dropFrameWhenSlow = -1; // drop frames when cpu is too slow
    int showMode = SHOW_MODE_NONE; // select show mode (0 = video, 1 = waves, 2 = RDFT)
    char *inputFormatName = nullptr;
    char *forceAudioCodecName = nullptr; // force decoder
    char *forceSubtitleCodecName = nullptr; // force decoder
    char *forceVideoCodecName = nullptr; // force decoder
    double rdftSpeed = 0.02;  // rdft speed
    int infiniteBuffer = -1; // don't limit the input buffer size (useful with realtime streams)
    int showStatus = 1; // show status
    int findStreamInfo = 1; // read and decode the streams to fill missing information with heuristics

    void showOptions();

};

#endif //SPLAYER_MAC_OPTIONS_H
