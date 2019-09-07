#ifndef SPLAYER_MAC_DEMO_SDLVIDEODEVICE_H
#define SPLAYER_MAC_DEMO_SDLVIDEODEVICE_H

#include <device/VideoDevice.h>
#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL.h>
#include <queue/FrameQueue.h>

/// 一秒
/// 1 second
#define CURSOR_HIDE_DELAY                   (1000000*1.0F)

class SDLVideoDevice : public VideoDevice {

    const char *const TAG = "VideoDevice";

public:

    /// 窗口无边框
    int borderLess = 0; // borderLess window

    int alwaysOntop = 0;

    /// 光标是否隐藏
    int cursorHidden = 0;

    /// 光标最后显示时间
    int64_t cursorLastShown = 0;

    /// 是否全屏
    int isFullScreen = 0;

    /// 设置窗口
    bool isDisplayWindow;

    /// 默认宽度
    int defaultWidth = 640;

    /// 默认高度
    int defaultHeight = 480;

    /// 画布宽度
    int surfaceWidth = 0;

    /// 画布高度
    int surfaceHeight = 0;

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

    int onInitTexture(int initTexture, int width, int height, TextureFormat format, BlendMode blendMode,
                      int rotate) override;

    int onUpdateYUV(uint8_t *yData, int yPitch, uint8_t *uData, int uPitch, uint8_t *vData, int vPitch) override;

    int onUpdateARGB(uint8_t *rgba, int pitch) override;

    void onRequestRenderStart(Frame *frame) override;

    void terminate() override;

    int onRequestRenderEnd(Frame *frame, bool flip) override;

    void setYuvConversionMode(AVFrame *frame);

    void calculateDisplayRect(SDL_Rect *rect,
                              int xLeft, int yTop,
                              int srcWidth, int scrHeight,
                              int picWidth, int picHeight,
                              AVRational picSar);

    SDL_BlendMode getSDLBlendMode(BlendMode mode);

    TextureFormat getTextureFormat(Uint32 format);

    void displayWindow();

    void setSurfaceSize(Frame *pFrame);


    Uint32 getSDLFormat(TextureFormat format);

    void destroyVideoTexture();


    void toggleFullScreen();
};


#endif
