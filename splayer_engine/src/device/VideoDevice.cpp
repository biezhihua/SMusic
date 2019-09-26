#include <device/VideoDevice.h>

VideoDevice::VideoDevice() {}

VideoDevice::~VideoDevice() {}

int VideoDevice::onInitTexture(int initTexture, int newWidth, int newHeight,
                               TextureFormat format, BlendMode blendMode,
                               int rotate) {
    return 0;
}

int VideoDevice::onUpdateYUV(uint8_t *yData, int yPitch, uint8_t *uData,
                             int uPitch, uint8_t *vData, int vPitch) {
    return 0;
}

int VideoDevice::onUpdateARGB(uint8_t *rgba, int pitch) { return 0; }

int VideoDevice::onRequestRenderEnd(Frame *frame, bool flip) { return 0; }

TextureFormat VideoDevice::getTextureFormat(int format) {
    switch (format) {
        case AV_PIX_FMT_RGB8:
            return FMT_RGB8;
        case AV_PIX_FMT_RGB444:
            return FMT_RGB444;
        case AV_PIX_FMT_RGB555:
            return FMT_RGB555;
        case AV_PIX_FMT_BGR555:
            return FMT_BGR555;
        case AV_PIX_FMT_RGB565:
            return FMT_RGB565;
        case AV_PIX_FMT_BGR565:
            return FMT_BGR565;
        case AV_PIX_FMT_RGB24:
            return FMT_RGB24;
        case AV_PIX_FMT_BGR24:
            return FMT_BGR24;
        case AV_PIX_FMT_0RGB32:
            return FMT_0RGB32;
        case AV_PIX_FMT_0BGR32:
            return FMT_0BGR32;
        case AV_PIX_FMT_NE(RGB0, 0BGR):
            return FMT_NE_RGBX;
        case AV_PIX_FMT_NE(BGR0, 0RGB):
            return FMT_NE_BGRX;
        case AV_PIX_FMT_RGB32:
            return FMT_RGB32;
        case AV_PIX_FMT_RGB32_1:
            return FMT_RGB32_1;
        case AV_PIX_FMT_BGR32:
            return FMT_BGR32;
        case AV_PIX_FMT_BGR32_1:
            return FMT_BGR32_1;
        case AV_PIX_FMT_YUV420P:
            return FMT_YUV420P;
        case AV_PIX_FMT_YUYV422:
            return FMT_YUYV422;
        case AV_PIX_FMT_UYVY422:
            return FMT_UYVY422;
        case AV_PIX_FMT_NONE:
            return FMT_NONE;
        default:
            return FMT_NONE;
    }
}

BlendMode VideoDevice::getBlendMode(TextureFormat format) {
    if (format == FMT_RGB32 || format == FMT_RGB32_1 || format == FMT_BGR32 ||
        format == FMT_BGR32_1) {
        return BLEND_BLEND;
    }
    return BLEND_NONE;
}

void VideoDevice::onRequestRenderStart(Frame *frame) {

}

void VideoDevice::setPlayerState(PlayerState *playerState) {
    this->playerState = playerState;
}

int VideoDevice::create() { return 0; }

int VideoDevice::destroy() { return 0; }
