#ifndef SPLAYER_MAC_DEMO_MACMEDIASYNC_H
#define SPLAYER_MAC_DEMO_MACMEDIASYNC_H

#include <sync/MediaSync.h>
#include <SDL.h>
#include <SDLVideoDevice.h>

class SDLMediaSync : public MediaSync {

    const char *const TAG = "SDLMediaSync";

public:
    void run() override;

private:

    int64_t lastMouseLeftClick = 0;

    void doKeySystem(const SDL_Event &event);

    bool isNotHaveWindow() const;

    bool isQuitKey(const SDL_Event &event) const;

public:

    void doWindowEvent(const SDL_Event &event);

    void showCursor() const;

    void hideCursor() const;

    int isFullScreenClick();

    void doExit();

    void doSeek(double increment) const;
};


#endif //SPLAYER_MAC_DEMO_MACMEDIASYNC_H
