#ifndef SPLAYER_MAC_DEMO_MACEVENT_H
#define SPLAYER_MAC_DEMO_MACEVENT_H

#include "Event.h"
#include <SDL.h>
#include "MacOptions.h"
#include "MacSurface.h"
#include "MacStream.h"

#define MAC_EVENT_TAG "MacEvent"

class MacEvent : public Event {

private:

    int64_t lastMouseLeftClick = 0;

    void doKeySystem(const SDL_Event &event) const;

    bool isNotHaveWindow() const;

    bool isQuitKey(const SDL_Event &event) const;

public:

    int eventLoop();

    void doWindowEvent(const SDL_Event &event);

    void showCursor() const;

    void hideCursor() const;

    int isFullScreenClick();

    void doExit();

    void doSeek(double increment) const;

};


#endif //SPLAYER_MAC_DEMO_MACEVENT_H
