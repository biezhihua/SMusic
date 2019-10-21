#ifndef SPLAYER_ANDROID_GLESVIDEODEVICE_H
#define SPLAYER_ANDROID_GLESVIDEODEVICE_H

#include <device/VideoDevice.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include <EglHelper.h>

class GLESVideoDevice : VideoDevice {

    const char *const TAG = "GLESVideoDevice";

private:

    /// Surface窗口
    ANativeWindow *window = nullptr;

    /// EGL帮助器
    EglHelper *eglHelper = nullptr;

    /// eglSurface
    EGLSurface eglSurface = nullptr;

    /// 重新设置Surface
    bool surfaceReset;

    /// 是否存在Surface
    bool hasSurface;

    /// 窗口宽度
    int surfaceWidth;

    /// 窗口高度
    int surfaceHeight;

    /// EGLSurface
    bool haveEGLSurface;

    /// 释放资源
    bool haveEGlContext;

public:

    GLESVideoDevice();

    ~GLESVideoDevice() override;

    int create() override;

    int destroy() override;

    int onInitTexture(int initTexture, int newWidth, int newHeight, TextureFormat format,
                      BlendMode blendMode, int rotate) override;

    int onUpdateYUV(uint8_t *yData, int yPitch, uint8_t *uData, int uPitch, uint8_t *vData,
                    int vPitch) override;

    int onUpdateARGB(uint8_t *rgba, int pitch) override;

    void onRequestRenderStart(Frame *frame) override;

    int onRequestRenderEnd(Frame *frame, bool flip) override;

    TextureFormat getTextureFormat(int format) override;

    BlendMode getBlendMode(TextureFormat format) override;

    int onSurfaceCreated(ANativeWindow *nativeWindow);
};


#endif
