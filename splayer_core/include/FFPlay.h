#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#ifndef SPLAYER_PLAY_H
#define SPLAYER_PLAY_H


#include "Log.h"
#include "Define.h"
#include "Mutex.h"
#include "MessageQueue.h"
#include "AOut.h"
#include "VOut.h"
#include "Pipeline.h"
#include "State.h"
#include "Error.h"
#include "VideoState.h"
#include "Thread.h"

extern "C" {
#include <libavutil/time.h>
#include <libavutil/rational.h>
#include <libavutil/mem.h>
#include <libavutil/log.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/avutil.h>
#include <libavutil/dict.h>
#include <libavutil/avstring.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include "Cmdutils.h"
};

class FFPlay {

private:

    Mutex *avMutex = nullptr;
    Mutex *vfMutex = nullptr;

    /**
     * Message Loop
     */
    MessageQueue *msgQueue = nullptr;

    /**
     * Audio Output
     */
    AOut *aOut = nullptr;

    /**
     * Video Output
     */
    VOut *vOut = nullptr;

    /**
     * Data input
     */
    Pipeline *pipeline = nullptr;

    /**
     * format/codec options
     */
    AVDictionary *formatOpts = nullptr;
    AVDictionary *codecOpts = nullptr;
    AVDictionary *swsDict = nullptr;
    AVDictionary *swrOpts = nullptr;
    AVDictionary *swrPresetOpts = nullptr;
    AVDictionary *playerOpts = nullptr;

    AVInputFormat *inputFormat = nullptr;
    char *inputFileName = nullptr;

    VideoState *videoState = nullptr;

    int videoQueueSize;

    int startupVolume = 100;

    int avSyncType = AV_SYNC_AUDIO_MASTER;

    char *inputFormatName;

    int skipCalcFrameRate;

    int genpts;

    int findStreamInfo;

    int seekByBytes;

    char *windowTitle;

    int64_t startTime = AV_NOPTS_VALUE;

    int64_t duration = AV_NOPTS_VALUE;

    char *wantedStreamSpec[AVMEDIA_TYPE_NB] = {0};

    int audioDisable;

    int videoDisable;

    int subtitleDisable;

    int showMode = SHOW_MODE_NONE;

    int infiniteBuffer = -1;

    int renderWaitStart;

    int startOnPrepared;

    bool prepared;

    int autoResume;

    int seekAtStart;

    AVPacket flushPacket;

    int loop = 1;

    int autoExit;

public:
    FFPlay();

    ~FFPlay();

    void setAOut(AOut *aOut);

    void setVOut(VOut *vOut);

    void setPipeline(Pipeline *pipeline);

    MessageQueue *getMsgQueue() const;

    int stop();

    int shutdown();

    int waitStop();

    int prepareAsync(const char *fileName);

    int getMsg(Message *pMessage, bool block);

    /**
     * this thread gets the stream from the disk or the network
     */
    int readThread();

private:
    void showVersionsAndOptions();

    void showDict(const char *tag, AVDictionary *dict);

    VideoState *streamOpen();

    void streamClose();

    int initClock(Clock *pClock, int *pQueueSerial);

    void setClock(Clock *pClock, double pts, int serial);

    void setClockAt(Clock *pClock, double pts, int serial, double time);

    int getStartupVolume();

    int isRealTime(AVFormatContext *pContext);

    int streamComponentOpen(int streamIndex);

    int streamHasEnoughPackets(AVStream *pStream, int index, PacketQueue *pQueue);

    int frameQueueNbRemaining(FrameQueue *pQueue);

    void streamComponentClose(AVStream *pStream);

    int frameQueueInit(FrameQueue *pFrameQueue, PacketQueue *pPacketQueue, int queueSize, int keepLast);

    int frameQueueDestroy(FrameQueue *pFrameQueue);
};

#endif //SPLAYER_PLAY_H

#pragma clang diagnostic pop