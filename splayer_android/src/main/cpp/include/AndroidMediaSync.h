#ifndef SPLAYER_ANDROID_SYNC_H
#define SPLAYER_ANDROID_SYNC_H

#include <sync/MediaSync.h>

class AndroidMediaSync : public MediaSync {

    const char *const TAG = "AndroidMediaSync";
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
