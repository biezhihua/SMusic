#ifndef SPLAYER_MAC_DEMO_MESSAGE_H
#define SPLAYER_MAC_DEMO_MESSAGE_H

#include <common/Thread.h>
#include <message/IMessageListener.h>
#include <message/MessageQueue.h>

class MessageCenter : public Runnable {

    const char *const TAG = "MessageCenter";

private:

    Msg msg;

    bool abortRequest = false;

    Mutex mutex;
    Condition condition;

    /// 消息线程
    Thread *msgThread = nullptr;

protected:

    MessageQueue *msgQueue = nullptr;

    IMessageListener *msgListener = nullptr;

public:

    MessageCenter();

    ~MessageCenter() override;

    void run() override;

    void setMsgListener(IMessageListener *msgListener);

    int start();

    int stop();

    MessageQueue *getMsgQueue();

    void startMsgQueue();

    void stopMsgQueue();

    int notifyMsg(int what);

    int notifyMsg(int what, int arg1);

    int notifyMsg(int what, int arg1, int arg2);

    int removeMsg(int what);

    void executeMsg(bool block);
};


#endif //SPLAYER_MAC_DEMO_MESSAGE_H
