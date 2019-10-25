#include "AndroidVideoDevice.h"

AndroidVideoDevice::AndroidVideoDevice() : VideoDevice() {

}

AndroidVideoDevice::~AndroidVideoDevice() = default;

int AndroidVideoDevice::create() {
    if (DEBUG) {
        ALOGD(TAG, __func__);
    }
    window = nullptr;
    windowWidth = 0;
    windowHeight = 0;
    eglSurface = EGL_NO_SURFACE;
    eglHelper = new EglHelper();
    haveEGLSurface = false;
    haveEGlContext = false;
    hasWindow = false;
    videoTexture = (Texture *) malloc(sizeof(Texture));
    memset(videoTexture, 0, sizeof(Texture));
    renderNode = nullptr;
    resetVertices();
    resetTexVertices();
    return SUCCESS;
}

int AndroidVideoDevice::destroy() {
    if (DEBUG) {
        ALOGD(TAG, __func__);
    }
    destroy(true);

    delete videoTexture;
    videoTexture = nullptr;

    delete eglHelper;
    eglHelper = nullptr;
    return SUCCESS;
}

int AndroidVideoDevice::onInitTexture(int initTexture,
                                      int newWidth, int newHeight,
                                      TextureFormat format,
                                      BlendMode blendMode,
                                      int rotate) {

    mutex.lock();

    if (!haveEGlContext) {
        haveEGlContext = eglHelper->init(FLAG_TRY_GLES3) == SUCCESS;
        if (DEBUG) {
            ALOGD(TAG, "%s haveEGlContext = %d", __func__, haveEGlContext);
        }
    }

    if (!haveEGlContext) {
        ALOGE(TAG, "%s no have egl context", __func__);
        mutex.unlock();
        return ERROR;
    }

    // 重新设置Surface，兼容SurfaceHolder处理
    if (hasWindow && windowReset) {
        destroy(false);
        windowReset = false;
    }

    // 创建/释放EGLSurface
    if (eglSurface == EGL_NO_SURFACE && window != nullptr) {
        if (hasWindow && !haveEGLSurface) {
            eglSurface = eglHelper->createSurface(window);
            if (eglSurface != EGL_NO_SURFACE) {
                haveEGLSurface = true;
                if (DEBUG) {
                    ALOGD(TAG, "%s haveEGLSurface = %d", __func__, haveEGLSurface);
                }
            }
        }
    } else if (eglSurface != EGL_NO_SURFACE && haveEGLSurface) {
        // 处于SurfaceDestroyed状态，释放EGLSurface
        if (!hasWindow) {
            destroy(false);
        }
    }

    // 计算帧的宽高，如果不相等，则需要重新计算缓冲区的大小
    if (window != nullptr && windowWidth != 0 && windowHeight != 0) {
        // 宽高比例不一致时，需要调整缓冲区的大小，这里是以宽度为基准
        if ((windowWidth / windowHeight) != (newWidth / newHeight)) {
            windowHeight = windowWidth * newHeight / newWidth;
            int windowFormat = ANativeWindow_getFormat(window);
            ANativeWindow_setBuffersGeometry(window, windowWidth, windowHeight, windowFormat);
        }
    }

    videoTexture->rotate = rotate;
    videoTexture->frameWidth = newWidth;
    videoTexture->frameHeight = newHeight;
    videoTexture->height = newHeight;
    videoTexture->format = format;
    videoTexture->blendMode = blendMode;
    videoTexture->direction = FLIP_NONE;
    eglHelper->makeCurrent(eglSurface);
    if (renderNode == nullptr) {
        renderNode = new InputRenderNode();
        if (renderNode != nullptr) {
            renderNode->initFilter(videoTexture);
        }
    }
    mutex.unlock();
    return SUCCESS;
}

int
AndroidVideoDevice::onUpdateYUV(uint8_t *yData, int yPitch,
                                uint8_t *uData, int uPitch,
                                uint8_t *vData, int vPitch) {
    if (!haveEGlContext) {
        return ERROR;
    }
    mutex.lock();
    videoTexture->pitches[0] = yPitch;
    videoTexture->pitches[1] = uPitch;
    videoTexture->pitches[2] = vPitch;
    videoTexture->pixels[0] = yData;
    videoTexture->pixels[1] = uData;
    videoTexture->pixels[2] = vData;
    if (renderNode != nullptr && eglSurface != EGL_NO_SURFACE) {
        eglHelper->makeCurrent(eglSurface);
        renderNode->uploadTexture(videoTexture);
    }
    // 设置像素实际的宽度，即linesize的值
    videoTexture->width = yPitch;
    mutex.unlock();
    return SUCCESS;
}

int AndroidVideoDevice::onUpdateARGB(uint8_t *rgba, int pitch) {
    if (!haveEGlContext) {
        return ERROR;
    }
    mutex.lock();
    videoTexture->pitches[0] = pitch;
    videoTexture->pixels[0] = rgba;
    if (renderNode != nullptr && eglSurface != EGL_NO_SURFACE) {
        eglHelper->makeCurrent(eglSurface);
        renderNode->uploadTexture(videoTexture);
    }
    // 设置像素实际的宽度，即linesize的值
    videoTexture->width = pitch / 4;
    mutex.unlock();
    return SUCCESS;
}

void AndroidVideoDevice::onRequestRenderStart(Frame *frame) {
    if (DEBUG) {
        ALOGD(TAG, "%s frame=%p", __func__, frame);
    }
}

int AndroidVideoDevice::onRequestRenderEnd(Frame *frame, bool flip) {
    if (DEBUG) {
        ALOGD(TAG, "%s frame=%p flip=%d", __func__, frame, flip);
    }
    if (!haveEGlContext) {
        return -1;
    }
    mutex.lock();
    videoTexture->direction = flip ? FLIP_VERTICAL : FLIP_NONE;
    if (renderNode != nullptr && eglSurface != EGL_NO_SURFACE) {
        eglHelper->makeCurrent(eglSurface);
        if (windowWidth != 0 && windowHeight != 0) {
            renderNode->setDisplaySize(windowWidth, windowHeight);
        }
        renderNode->drawFrame(videoTexture);
        eglHelper->swapBuffers(eglSurface);
    }
    mutex.unlock();
    return SUCCESS;
}

TextureFormat AndroidVideoDevice::getTextureFormat(int format) {
    return VideoDevice::getTextureFormat(format);
}

BlendMode AndroidVideoDevice::getBlendMode(TextureFormat format) {
    return VideoDevice::getBlendMode(format);
}

int AndroidVideoDevice::setNativeWindow(ANativeWindow *nativeWindow) {
    mutex.lock();
    if (window != nullptr) {
        ANativeWindow_release(window);
        window = nullptr;
        windowReset = true;
    }
    window = nativeWindow;
    if (window != nullptr) {
        windowWidth = ANativeWindow_getWidth(window);
        windowHeight = ANativeWindow_getHeight(window);
    }
    hasWindow = true;
    mutex.unlock();
    return SUCCESS;
}

void AndroidVideoDevice::destroy(bool releaseContext) {
    if (eglSurface != EGL_NO_SURFACE) {
        eglHelper->destroySurface(eglSurface);
        eglSurface = EGL_NO_SURFACE;
        haveEGLSurface = false;
    }
    if (eglHelper->getEglContext() != EGL_NO_CONTEXT && releaseContext) {
        if (renderNode) {
            renderNode->destroy();
            delete renderNode;
        }
        eglHelper->release();
        haveEGlContext = false;
    }
}

void AndroidVideoDevice::resetVertices() {
    const float *vertexCoordinates = CoordinateUtils::getVertexCoordinates();
    for (int i = 0; i < COORDINATES_SIZE; ++i) {
        vertices[i] = vertexCoordinates[i];
    }
}

void AndroidVideoDevice::resetTexVertices() {
    const float *vertices = CoordinateUtils::getTextureCoordinates(ROTATE_NONE);
    for (int i = 0; i < COORDINATES_SIZE; ++i) {
        textureVertices[i] = vertices[i];
    }
}

