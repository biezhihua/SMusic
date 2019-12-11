#ifndef SPLAYER_IOS_MEDIASYNC_H
#define SPLAYER_IOS_MEDIASYNC_H

#include "MediaSync.h"

class iOSMediaSync : public MediaSync {

    const char *const TAG = "[MP][ANDROID][MediaSync]";
private:

    /// 同步线程
    Thread *syncThread = nullptr;

    /// 退出标记
    bool isQuit;

public:

private:

    void start(VideoDecoder *videoDecoder, AudioDecoder *audioDecoder) override;

    void stop() override;

    void run() override;
};


#endif
