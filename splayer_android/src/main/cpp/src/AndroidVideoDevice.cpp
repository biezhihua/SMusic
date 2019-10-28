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
            ALOGD(TAG, "[%s] haveEGlContext = %d", __func__, haveEGlContext);
        }
    }

    if (!haveEGlContext) {
        ALOGE(TAG, "[%s] no have egl context", __func__);
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
                    ALOGD(TAG, "[%s] haveEGLSurface = %d", __func__, haveEGLSurface);
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
        if ((windowWidth * 1.0F / windowHeight) != (newWidth * 1.0F / newHeight)) {
            windowHeight = (int) (windowWidth * newHeight * 1.0F / newWidth);
            int windowFormat = ANativeWindow_getFormat(window);
            ANativeWindow_setBuffersGeometry(window, windowWidth, windowHeight, windowFormat);
        }
    }

    //
    videoTexture->rotate = rotate;
    videoTexture->frameWidth = newWidth;
    videoTexture->frameHeight = newHeight;
    videoTexture->height = newHeight;
    videoTexture->format = format;
    videoTexture->blendMode = blendMode;
    videoTexture->direction = FLIP_NONE;

    // eglMakeCurrent把context绑定到当前的渲染线程以及draw和read指定的Surface。
    // draw用于除数据回读(glReadPixels、glCopyTexImage2D和glCopyTexSubImage2D)之外的所有GL 操作。
    // 回读操作作用于read指定的Surface上的帧缓冲(frame buffer)。
    // 因此，当我们在线程T上调用GL 指令，OpenGL ES 会查询T线程绑定是哪个Context C，
    // 进而查询是哪个Surface draw和哪个Surface read绑定到了这个Context C上。
    eglHelper->makeCurrent(eglSurface);

    // 初始化输入渲染节点
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
    videoTexture->pitches[0] = (uint16_t) (yPitch);
    videoTexture->pitches[1] = (uint16_t) (uPitch);
    videoTexture->pitches[2] = (uint16_t) (vPitch);
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
    videoTexture->pitches[0] = static_cast<uint16_t>(pitch);
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
}

int AndroidVideoDevice::onRequestRenderEnd(Frame *frame, bool flip) {
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

        // 实际上，这里需要重点注意的是surface。
        // 如果这里的`surface 是一个像素缓冲(pixel buffer)Surface，那什么都不会发生，
        // 调用将正确的返回，不报任何错误。
        // 但如果surface是一个双重缓冲surface(大多数情况)，
        // 这个方法将会交换surface内部的前端缓冲(front-buffer)和后端缓冲(back-surface)。
        // 后端缓冲用于存储渲染结果，前端缓冲则用于底层窗口系统，底层窗口系统将缓冲中的颜色信息显示到设备上。
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

