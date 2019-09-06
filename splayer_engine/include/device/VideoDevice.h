#ifndef VIDEODEVICE_H
#define VIDEODEVICE_H

#include <player/PlayerState.h>
#include <renderer/GLInputFilter.h>
#include <queue/FrameQueue.h>

class VideoDevice {

    const char *const TAG = "VideoDevice";

protected:

    AVFormatContext *formatContext;

    PlayerState *playerState;

public:
    VideoDevice();

    virtual ~VideoDevice();

    virtual void terminate();

    // 初始化视频纹理
    virtual int onInitTexture(int initTexture,
                              int newWidth, int newHeight,
                              TextureFormat format, BlendMode blendMode,
                              int rotate = 0);

    // 更新YUV数据
    virtual int onUpdateYUV(uint8_t *yData, int yPitch, uint8_t *uData, int uPitch, uint8_t *vData, int vPitch);

    // 更新ARGB数据
    virtual int onUpdateARGB(uint8_t *rgba, int pitch);

    // 请求渲染
    virtual void onRequestRenderStart(Frame *frame);

    // 请求渲染
    virtual int onRequestRenderEnd(Frame *frame, bool flip);

    // 获取纹理格式
    virtual TextureFormat getTextureFormat(int format);

    // 获取混合模式
    virtual BlendMode getBlendMode(TextureFormat format);

    void setFormatContext(AVFormatContext *formatContext);

    void setPlayerState(PlayerState *playerState);


};

#endif //VIDEODEVICE_H
