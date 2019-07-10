#ifndef SPLAYER_PLAY_H
#define SPLAYER_PLAY_H

#include "Log.h"
#include "Mutex.h"
#include "MessageQueue.h"
#include "AOut.h"
#include "VOut.h"
#include "Pipeline.h"
#include "State.h"
#include "VideoState.h"
#include <libavformat/avformat.h>

class FFPlay {

private:
    Mutex *avMutex = nullptr;
    Mutex *vfMutex = nullptr;
    MessageQueue *msgQueue = nullptr;
    AOut *aOut = nullptr;
    VOut *vOut = nullptr;
    Pipeline *pipeline = nullptr;
    VideoState *videoState = nullptr;

    char *inputFileName = nullptr;

private:

    VideoState *streamOpen(const char *name, AVInputFormat *inputFormat);

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

};


#endif //SPLAYER_PLAY_H
