#ifndef SPLAYER_CORE_IMESSAGE_H
#define SPLAYER_CORE_IMESSAGE_H

class Event;

#include "Msg.h"
#include "Event.h"

class Message {

public:
    virtual void onMessage(Event *event, Msg *message) = 0;

};

#endif //SPLAYER_MAC_DEMO_IMESSAGE_H
