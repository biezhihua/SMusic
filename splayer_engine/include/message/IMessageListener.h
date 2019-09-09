#ifndef SPLAYER_MAC_DEMO_IMESSAGELISTENER_H
#define SPLAYER_MAC_DEMO_IMESSAGELISTENER_H

class Msg;

class IMessageListener {
public:
    virtual void onMessage(Msg *msg) = 0;
};

#endif //SPLAYER_MAC_DEMO_IMESSAGELISTENER_H
