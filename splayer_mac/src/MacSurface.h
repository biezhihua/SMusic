

#ifndef SPLAYER_COMMAND_MACVOUT_H
#define SPLAYER_COMMAND_MACVOUT_H

#include "Surface.h"
#include "Error.h"
#include <SDL.h>

extern "C" {
#include <libavutil/rational.h>
#include <libswscale/swscale.h>
};

#define MAC_SURFACE_TAG "MacSurface"

/**
 * https://github.com/Twinklebear/TwinklebearDev-Lessons
 * https://blog.csdn.net/leixiaohua1020/article/details/40680907
 */

static const struct TextureFormatEntry {
    enum AVPixelFormat format;
    int texture_fmt;
} sdl_texture_format_map[] = {
        {AV_PIX_FMT_RGB8,           SDL_PIXELFORMAT_RGB332},
        {AV_PIX_FMT_RGB444,         SDL_PIXELFORMAT_RGB444},
        {AV_PIX_FMT_RGB555,         SDL_PIXELFORMAT_RGB555},
        {AV_PIX_FMT_BGR555,         SDL_PIXELFORMAT_BGR555},
        {AV_PIX_FMT_RGB565,         SDL_PIXELFORMAT_RGB565},
        {AV_PIX_FMT_BGR565,         SDL_PIXELFORMAT_BGR565},
        {AV_PIX_FMT_RGB24,          SDL_PIXELFORMAT_RGB24},
        {AV_PIX_FMT_BGR24,          SDL_PIXELFORMAT_BGR24},
        {AV_PIX_FMT_0RGB32,         SDL_PIXELFORMAT_RGB888},
        {AV_PIX_FMT_0BGR32,         SDL_PIXELFORMAT_BGR888},
        {AV_PIX_FMT_NE(RGB0, 0BGR), SDL_PIXELFORMAT_RGBX8888},
        {AV_PIX_FMT_NE(BGR0, 0RGB), SDL_PIXELFORMAT_BGRX8888},
        {AV_PIX_FMT_RGB32,          SDL_PIXELFORMAT_ARGB8888},
        {AV_PIX_FMT_RGB32_1,        SDL_PIXELFORMAT_RGBA8888},
        {AV_PIX_FMT_BGR32,          SDL_PIXELFORMAT_ABGR8888},
        {AV_PIX_FMT_BGR32_1,        SDL_PIXELFORMAT_BGRA8888},
        {AV_PIX_FMT_YUV420P,        SDL_PIXELFORMAT_IYUV},
        {AV_PIX_FMT_YUYV422,        SDL_PIXELFORMAT_YUY2},
        {AV_PIX_FMT_UYVY422,        SDL_PIXELFORMAT_UYVY},
        {AV_PIX_FMT_NONE,           SDL_PIXELFORMAT_UNKNOWN},
};

class MacSurface : public Surface {

private:
    unsigned swsFlags = SWS_BICUBIC;

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_RendererInfo rendererInfo = {nullptr};

    SDL_Texture *videoTexture;
    SDL_Texture *subtitleTexture;
    SDL_Texture *visTexture;

public:
    int create() override;

    int destroy() override;

    void setWindowSize(int width, int height, AVRational rational) override;

    int eventLoop();

private:
    void calculateDisplayRect(SDL_Rect *rect, int scrXLeft, int scrYTop, int scrWidth, int scrHeight, int picWidth, int picHeight, AVRational picSar);

    void doKeySystem(const SDL_Event &event) const;

    bool isHaveWindow() const;

    bool isQuitKey(const SDL_Event &event) const;

public:
    void displayWindow(int width, int height) override;

    void displayVideoImage() override;

    void setYuvConversionMode(AVFrame *pFrame);

    int uploadTexture(AVFrame *frame, SwsContext *convertContext);

    void getSDLPixFmtAndBlendMode(int format, Uint32 *sdlPixFmt, SDL_BlendMode *sdlBlendMode);

    int reallocTexture(SDL_Texture **texture, Uint32 new_format, int new_width, int new_height, SDL_BlendMode blendmode, int init_texture);
};


#endif //SPLAYER_COMMAND_MACVOUT_H
