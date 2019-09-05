#ifndef SPLAYER_MAC_DEMO_MACMEDIASYNC_H
#define SPLAYER_MAC_DEMO_MACMEDIASYNC_H

#include <sync/MediaSync.h>
#include <SDL.h>

class SDLMediaSync : public MediaSync {

public:
    void run() override;
};


#endif //SPLAYER_MAC_DEMO_MACMEDIASYNC_H
