#include <AndroidVideoDevice.h>

AndroidVideoDevice::AndroidVideoDevice() : VideoDevice() {
    window = nullptr;
    surfaceWidth = 0;
    surfaceHeight = 0;
    eglSurface = EGL_NO_SURFACE;
    eglHelper = new AndroidEGLHelper();
    haveEGLSurface = false;
    haveEGlContext = false;
    hasSurface = false;
}

AndroidVideoDevice::~AndroidVideoDevice() {

}

int AndroidVideoDevice::create() {
    return VideoDevice::create();
}

int AndroidVideoDevice::destroy() {
    return VideoDevice::destroy();
}

int AndroidVideoDevice::onInitTexture(int initTexture,
                                      int newWidth, int newHeight,
                                      TextureFormat format, BlendMode blendMode,
                                      int rotate) {


    return SUCCESS;
}

int
AndroidVideoDevice::onUpdateYUV(uint8_t *yData, int yPitch, uint8_t *uData, int uPitch,
                                uint8_t *vData,
                                int vPitch) {
    return VideoDevice::onUpdateYUV(yData, yPitch, uData, uPitch, vData, vPitch);
}

int AndroidVideoDevice::onUpdateARGB(uint8_t *rgba, int pitch) {
    return VideoDevice::onUpdateARGB(rgba, pitch);
}

void AndroidVideoDevice::onRequestRenderStart(Frame *frame) {
    VideoDevice::onRequestRenderStart(frame);
}

int AndroidVideoDevice::onRequestRenderEnd(Frame *frame, bool flip) {
    return VideoDevice::onRequestRenderEnd(frame, flip);
}

TextureFormat AndroidVideoDevice::getTextureFormat(int format) {
    return VideoDevice::getTextureFormat(format);
}

BlendMode AndroidVideoDevice::getBlendMode(TextureFormat format) {
    return VideoDevice::getBlendMode(format);
}

int AndroidVideoDevice::onSurfaceCreated(ANativeWindow *nativeWindow) {
    mutex.lock();
    if (window != nullptr) {
        ANativeWindow_release(window);
        window = nullptr;
        surfaceReset = true;
    }
    window = nativeWindow;
    if (window != nullptr) {
        surfaceWidth = ANativeWindow_getWidth(window);
        surfaceHeight = ANativeWindow_getHeight(window);
    }
    hasSurface = true;
    mutex.unlock();
    return SUCCESS;
}

