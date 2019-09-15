#ifndef SPLAYER_MAC_DEMO_MESSAGE_H
#define SPLAYER_MAC_DEMO_MESSAGE_H

#include <common/Thread.h>
#include <message/IMessageListener.h>
#include <message/MessageQueue.h>

class MessageCenter : public Runnable {

    const char *const TAG = "MessageCenter";

private:

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

    void setMsgQueue(MessageQueue *msgQueue);

    void setMsgListener(IMessageListener *msgListener);

    int start();

    int stop();

    MessageQueue *getMsgQueue() const;

    void startMsgQueue();

    void stopMsgQueue();
};


#endif //SPLAYER_MAC_DEMO_MESSAGE_H
