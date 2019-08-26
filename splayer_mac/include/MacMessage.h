#ifndef SPLAYER_MAC_MACMESSAGE_H
#define SPLAYER_MAC_MACMESSAGE_H

#include "Message.h"
#include "MacEvent.h"

class MacMessage : public Message {

public:
    void onMessage(Event *event, Msg *message) override;
};

#endif //SPLAYER_MAC_DEMO_MACMESSAGE_H
