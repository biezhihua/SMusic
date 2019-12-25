#include "iOSVideoDevice.h"

iOSVideoDevice::iOSVideoDevice() {

}

iOSVideoDevice::~iOSVideoDevice() {

}

int iOSVideoDevice::create() {
    return VideoDevice::create();
}

int iOSVideoDevice::destroy() {
    return VideoDevice::destroy();
}

int iOSVideoDevice::onInitTexture(int initTexture, int newWidth, int newHeight, TextureFormat format, BlendMode blendMode, int rotate) {
    return VideoDevice::onInitTexture(initTexture, newWidth, newHeight, format, blendMode, rotate);
}

int iOSVideoDevice::onUpdateYUV(uint8_t *yData, int yPitch, uint8_t *uData, int uPitch, uint8_t *vData, int vPitch) {
    ALOGD(TAG, "[%s] onUpdateYUV", __func__);
    return VideoDevice::onUpdateYUV(yData, yPitch, uData, uPitch, vData, vPitch);
}

int iOSVideoDevice::onUpdateARGB(uint8_t *rgba, int pitch) {
    return VideoDevice::onUpdateARGB(rgba, pitch);
}

void iOSVideoDevice::onRequestRenderStart(Frame *frame) {
    VideoDevice::onRequestRenderStart(frame);
}

int iOSVideoDevice::onRequestRenderEnd(Frame *frame, bool flip) {
    return VideoDevice::onRequestRenderEnd(frame, flip);
}

TextureFormat iOSVideoDevice::getTextureFormat(int format) {
    return VideoDevice::getTextureFormat(format);
}

BlendMode iOSVideoDevice::getBlendMode(TextureFormat format) {
    return VideoDevice::getBlendMode(format);
}

