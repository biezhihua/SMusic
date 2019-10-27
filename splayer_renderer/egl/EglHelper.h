#ifndef RENDERER_EGLHELPER_H
#define RENDERER_EGLHELPER_H

#ifdef __APPLE__
#else

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

#endif

#include "EglContext.h"
#include "utils/Log.h"

/**
 * Constructor flag:
 * surface must be recordable.  This discourages EGL from using a
 * pixel format that cannot be converted efficiently to something usable by the video
 * encoder.
 */
#define FLAG_RECORDABLE 0x01

/**
 * Constructor flag:
 * ask for GLES3, fall back to GLES2 if not available.
 * Without this flag, GLES2 is used.
 */
#define FLAG_TRY_GLES3 002

typedef EGLBoolean (EGLAPIENTRYP EGL_PRESENTATION_TIME_ANDROIDPROC)(EGLDisplay display,
                                                                    EGLSurface surface,
                                                                    khronos_stime_nanoseconds_t time);

class EglHelper {

    const char *const TAG = "[MP][RENDER][EglHelper]";

public:

    EglHelper();

    virtual ~EglHelper();

    /// 初始化
    int init(int flags);

    /// 初始化EGLDisplay、EGLContext、EGLConfig等资源
    int init(EGLContext sharedContext, int flags);

    /// 释放资源
    void release();

    /// 获取EglContext
    EGLContext getEglContext();

    /// 销毁Surface
    void destroySurface(EGLSurface eglSurface);

    /// 创建EGLSurface
    EGLSurface createSurface(ANativeWindow *surface);

    /// 创建离屏EGLSurface
    EGLSurface createSurface(int width, int height);

    /// 切换到当前上下文
    void makeCurrent(EGLSurface eglSurface);

    /// 切换到某个上下文
    void makeCurrent(EGLSurface drawSurface, EGLSurface readSurface);

    /// 没有上下文
    void makeNothingCurrent();

    /// 交换显示
    int swapBuffers(EGLSurface eglSurface);

    /// 设置pts
    void setPresentationTime(EGLSurface eglSurface, long nsecs);

    /// 判断是否属于当前上下文
    bool isCurrent(EGLSurface eglSurface);

    /// 执行查询
    int querySurface(EGLSurface eglSurface, int what);

    /// 查询字符串
    const char *queryString(int what);

    /// 获取当前的GLES 版本号
    int getGlVersion();

    /// 检查是否出错
    void checkEglError(const char *msg);

    /// 查询Surface宽度
    int getSurfaceWidth(EGLSurface surface);

    /// 查询Surface高度
    int getSurfaceHeight(EGLSurface surface);

private:

    /// 查找合适的EGLConfig
    EGLConfig getConfig(int flags, int version);

private:

    /// 是对实际显示设备的抽象
    EGLDisplay eglDisplay = nullptr;

    EGLConfig eglConfig = nullptr;

    EGLContext eglContext = nullptr;

    int glVersion;

    // 设置时间戳方法
    EGL_PRESENTATION_TIME_ANDROIDPROC eglPresentationTimeANDROID = nullptr;
};


#endif
