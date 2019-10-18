#ifndef ENGINE_MESSAGE_LISTENER_H
#define ENGINE_MESSAGE_LISTENER_H

class Msg;

class IMessageListener {
public:
    virtual void onMessage(Msg *msg) = 0;
};

#endif
