#ifndef RENDERER_EGLHELPER_H
#define RENDERER_EGLHELPER_H

#ifdef __APPLE__
#else

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

#endif

#include "EglContext.h"
#include "Log.h"

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

/**
 * 使用EGL的绘图的一般步骤：

 *      1. 获取 EGL Display 对象：eglGetDisplay()
 *      2. 初始化与 EGLDisplay 之间的连接：eglInitialize()
 *      3. 获取 EGLConfig 对象：eglChooseConfig()
 *      4. 创建 EGLContext 实例：eglCreateContext()
 *      5. 创建 EGLSurface 实例：eglCreateWindowSurface()
 *      6. 连接 EGLContext 和 EGLSurface：eglMakeCurrent()
 *      7. 使用 OpenGL ES API 绘制图形：gl_*()
 *      8. 切换 front buffer 和 back buffer 送显：eglSwapBuffer()
 *      9. 断开并释放与 EGLSurface 关联的 EGLContext 对象：eglRelease()
 *      10.删除 EGLSurface 对象
 *      11.删除 EGLContext 对象
 *      12.终止与 EGLDisplay 之间的连接
 *
 * https://woshijpf.github.io/android/2017/09/04/Android%E7%B3%BB%E7%BB%9F%E5%9B%BE%E5%BD%A2%E6%A0%88OpenGLES%E5%92%8CEGL%E4%BB%8B%E7%BB%8D.html
 * https://www.khronos.org/registry/EGL/sdk/docs/man/html/eglGetDisplay.xhtml
 * http://sr1.me/lets-talk-about-eglmakecurrent-eglswapbuffers-glflush-glfinish-chinese
 * https://katatunix.wordpress.com/2014/09/17/lets-talk-about-eglmakecurrent-eglswapbuffers-glflush-glfinish/
 */
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
