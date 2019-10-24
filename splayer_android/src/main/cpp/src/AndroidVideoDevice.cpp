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
    videoTexture = (Texture *) malloc(sizeof(Texture));
    memset(videoTexture, 0, sizeof(Texture));
//    renderNode = nullptr;
//    resetVertices();
//    resetTexVertices();
}

AndroidVideoDevice::~AndroidVideoDevice() {
    delete videoTexture;
    videoTexture = nullptr;

    delete eglHelper;
    eglHelper = nullptr;
}

int AndroidVideoDevice::create() {
    return VideoDevice::create();
}

int AndroidVideoDevice::destroy() {
    destroy(true);
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
    if (hasSurface && surfaceReset) {
        destroy(false);
        surfaceReset = false;
    }


    // 创建/释放EGLSurface
    if (eglSurface == EGL_NO_SURFACE && window != nullptr) {
        if (hasSurface && !haveEGLSurface) {
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
        if (!hasSurface) {
            destroy(false);
        }
    }

    // 计算帧的宽高，如果不相等，则需要重新计算缓冲区的大小
    if (window != nullptr && surfaceWidth != 0 && surfaceHeight != 0) {
        // 宽高比例不一致时，需要调整缓冲区的大小，这里是以宽度为基准
        if ((surfaceWidth / surfaceHeight) != (newWidth / newHeight)) {
            surfaceHeight = surfaceWidth * newHeight / newWidth;
            int windowFormat = ANativeWindow_getFormat(window);
            ANativeWindow_setBuffersGeometry(window, surfaceWidth, surfaceHeight, windowFormat);
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
//    if (renderNode != NULL && eglSurface != EGL_NO_SURFACE) {
//        eglHelper->makeCurrent(eglSurface);
//        renderNode->uploadTexture(videoTexture);
//    }
    // 设置像素实际的宽度，即linesize的值
    videoTexture->width = yPitch;
    mutex.unlock();
    return SUCCESS;
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

void AndroidVideoDevice::destroy(bool releaseContext) {
    if (eglSurface != EGL_NO_SURFACE) {
        eglHelper->destroySurface(eglSurface);
        eglSurface = EGL_NO_SURFACE;
        haveEGLSurface = false;
    }
    if (eglHelper->getEglContext() != EGL_NO_CONTEXT && releaseContext) {
//        if (renderNode) {
//            renderNode->destroy();
//            delete renderNode;
//        }
        eglHelper->release();
        haveEGlContext = false;
    }
}

//void AndroidVideoDevice::resetVertices() {
//    const float *verticesCoord = CoordinateUtils::getVertexCoordinates();
//    for (int i = 0; i < 8; ++i) {
//        vertices[i] = verticesCoord[i];
//    }
//}

//void AndroidVideoDevice::resetTexVertices() {
//    const float *vertices = CoordinateUtils::getTextureCoordinates(ROTATE_NONE);
//    for (int i = 0; i < 8; ++i) {
//        textureVertices[i] = vertices[i];
//    }
//}

