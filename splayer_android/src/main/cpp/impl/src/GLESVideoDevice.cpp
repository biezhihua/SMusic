#include <GLESVideoDevice.h>

GLESVideoDevice::GLESVideoDevice() : VideoDevice() {
    window = nullptr;
    surfaceWidth = 0;
    surfaceHeight = 0;
    eglSurface = EGL_NO_SURFACE;
    eglHelper = new EglHelper();
    haveEGLSurface = false;
    haveEGlContext = false;
    hasSurface = false;
}

GLESVideoDevice::~GLESVideoDevice() {

}

int GLESVideoDevice::create() {
    return VideoDevice::create();
}

int GLESVideoDevice::destroy() {
    return VideoDevice::destroy();
}

int GLESVideoDevice::onInitTexture(int initTexture,
                               int newWidth, int newHeight,
                               TextureFormat format, BlendMode blendMode,
                               int rotate) {


    return SUCCESS;
}

int
GLESVideoDevice::onUpdateYUV(uint8_t *yData, int yPitch, uint8_t *uData, int uPitch, uint8_t *vData,
                             int vPitch) {
    return VideoDevice::onUpdateYUV(yData, yPitch, uData, uPitch, vData, vPitch);
}

int GLESVideoDevice::onUpdateARGB(uint8_t *rgba, int pitch) {
    return VideoDevice::onUpdateARGB(rgba, pitch);
}

void GLESVideoDevice::onRequestRenderStart(Frame *frame) {
    VideoDevice::onRequestRenderStart(frame);
}

int GLESVideoDevice::onRequestRenderEnd(Frame *frame, bool flip) {
    return VideoDevice::onRequestRenderEnd(frame, flip);
}

TextureFormat GLESVideoDevice::getTextureFormat(int format) {
    return VideoDevice::getTextureFormat(format);
}

BlendMode GLESVideoDevice::getBlendMode(TextureFormat format) {
    return VideoDevice::getBlendMode(format);
}

int GLESVideoDevice::onSurfaceCreated(ANativeWindow *nativeWindow) {
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

