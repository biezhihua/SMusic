

#ifndef SPLAYER_COMMAND_MACVOUT_H
#define SPLAYER_COMMAND_MACVOUT_H

#include "Surface.h"
#include "Error.h"
#include <SDL.h>

#define MAC_SURFACE_TAG "MacSurface"

/**
 * https://github.com/Twinklebear/TwinklebearDev-Lessons
 * https://blog.csdn.net/leixiaohua1020/article/details/40680907
 */
class MacSurface : public Surface {

private:
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_RendererInfo rendererInfo = {nullptr};

public:
    int create() override;

    int destroy() override;

    void setWindowTitle(char *title) override;
};


#endif //SPLAYER_COMMAND_MACVOUT_H
