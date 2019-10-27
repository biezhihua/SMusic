#ifndef ENGINE_MESSAGE_CENTER_H
#define ENGINE_MESSAGE_CENTER_H

#include "common/Thread.h"
#include "message/IMessageListener.h"
#include "message/MessageQueue.h"
#include "player/IMediaPlayer.h"
#include "player/ISyncMediaPlayer.h"

class MessageCenter : public Runnable {

    const char *const TAG = "[MP][NATIVE][MessageCenter]";

private:

    Msg msg;

    bool abortRequest = false;

    Thread *msgThread = nullptr;

    IMediaPlayer *mediaPlayer = nullptr;

    ISyncMediaPlayer *syncMediaPlayer = nullptr;

protected:

    MessageQueue *msgQueue = nullptr;

    IMessageListener *msgListener = nullptr;

public:
    MessageCenter(IMediaPlayer *mediaPlayer, ISyncMediaPlayer *innerMediaPlayer);

    ~MessageCenter() override;

    void run() override;

    void setMsgListener(IMessageListener *msgListener);

    int start();

    int stop();

    void startMsgQueue();

    void stopMsgQueue();

    int notifyMsg(int what);

    int notifyMsg(int what, int arg1);

    int notifyMsg(int what, float arg1);

    int notifyMsg(int what, int arg1, int arg2);

    int removeMsg(int what);

    void executeMsg(bool block);

    PlayerStatus getStatus(int arg);
};


#endif
