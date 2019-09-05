#ifndef SPLAYER_MAC_DEMO_SDLVIDEODEVICE_H
#define SPLAYER_MAC_DEMO_SDLVIDEODEVICE_H

#include <device/VideoDevice.h>
#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL.h>
#include <queue/FrameQueue.h>

class SDLVideoDevice : public VideoDevice {

    const char *const TAG = "VideoDevice";

private:

    /// 画布宽度
    int surfaceWidth = 640;

    /// 画布高度
    int surfaceHeight = 480;

    /// 画布X轴偏移位置
    int surfaceLeftOffset = 0;

    /// 画布Y轴偏移位置
    int surfaceTopOffset = 0;

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_RendererInfo rendererInfo = {nullptr};

    SDL_Texture *videoTexture;
    SDL_Texture *subtitleTexture;
    SDL_Texture *visTexture;

private:
    int create();

    int destroy();

public:

    SDLVideoDevice();

    ~SDLVideoDevice() override;

    int onInitTexture(int initTexture,int width, int height, TextureFormat format, BlendMode blendMode, int rotate) override;

    int onUpdateYUV(uint8_t *yData, int yPitch, uint8_t *uData, int uPitch, uint8_t *vData, int vPitch) override;

    int onUpdateARGB(uint8_t *rgba, int pitch) override;

    void onRequestRenderStart() override;

    void terminate() override;

    int onRequestRenderEnd(Frame *frame, bool flip) override;

    void setYuvConversionMode(AVFrame *frame);

    void calculateDisplayRect(SDL_Rect *rect,
                              int xLeft, int yTop,
                              int srcWidth, int scrHeight,
                              int picWidth, int picHeight,
                              AVRational picSar);

    SDL_BlendMode getSDLBlendMode(BlendMode mode);

    TextureFormat getSDLFormat(Uint32 format);
};


#endif
