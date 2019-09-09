#ifndef SPLAYER_MAC_DEMO_MESSAGE_H
#define SPLAYER_MAC_DEMO_MESSAGE_H

#include <common/Thread.h>
#include <message/IMessageListener.h>
#include <message/MessageQueue.h>

class MessageDevice : public Runnable {

    const char *const TAG = "MessageDevice";

public:

protected:

    MessageQueue *msgQueue;

    IMessageListener *msgListener;

public:

    MessageDevice();

    ~MessageDevice() override;

    void run() override;

    void setMsgQueue(MessageQueue *msgQueue);

    void setMsgListener(IMessageListener *msgListener);
};


#endif //SPLAYER_MAC_DEMO_MESSAGE_H
