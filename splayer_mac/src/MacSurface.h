

#ifndef SPLAYER_COMMAND_MACVOUT_H
#define SPLAYER_COMMAND_MACVOUT_H

#include "Surface.h"
#include "Error.h"
#include <SDL.h>

extern "C" {
#include <libavutil/rational.h>
};

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

    void setWindowSize(int width, int height, AVRational rational) override;

    int eventLoop();

private:
    void calculateDisplayRect(SDL_Rect *rect, int scrXLeft, int scrYTop, int scrWidth, int scrHeight, int picWidth, int picHeight, AVRational picSar);
};


#endif //SPLAYER_COMMAND_MACVOUT_H
