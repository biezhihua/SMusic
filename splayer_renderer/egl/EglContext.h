#ifndef RENDERER_EGLCONTEXT_H
#define RENDERER_EGLCONTEXT_H

#ifdef __APPLE__
#else

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

#endif

#include "EglHelper.h"
#include "utils/Log.h"
#include <mutex>

/**
 * EGLContext 上下文，为了方便使用SharedContext而造的
 * https://woshijpf.github.io/android/2017/09/04/Android%E7%B3%BB%E7%BB%9F%E5%9B%BE%E5%BD%A2%E6%A0%88OpenGLES%E5%92%8CEGL%E4%BB%8B%E7%BB%8D.html
 * https://www.khronos.org/registry/EGL/sdk/docs/man/html/eglGetDisplay.xhtml
 * http://sr1.me/lets-talk-about-eglmakecurrent-eglswapbuffers-glflush-glfinish-chinese
 * https://katatunix.wordpress.com/2014/09/17/lets-talk-about-eglmakecurrent-eglswapbuffers-glflush-glfinish/
 */
class EglContext {

    const char *const TAG = "[MP][RENDER][EglContext]";

public:

    static EglContext *getInstance();

    void destroy();

    EGLContext getContext();

private:

    static EglContext *instance;

    static std::mutex mutex;

    /// 内部状态信息(View port, depth range, clear color, textures, VBO, FBO, ...)
    /// 调用缓存，保存了在这个Context下发起的GL调用指令。(OpenGL 调用是异步的)
    EGLContext eglContext = nullptr;

    /// 用于连接设备上的底层窗口系统
    EGLDisplay eglDisplay = nullptr;

private:

    EglContext();

    virtual ~EglContext();

    int init(int flags);

    int release();

    EGLConfig getConfig(int flags, int version);

    void checkEglError(const char *msg);

    bool isGLES3(int flags) const;

    EGLContext createGLES3Context(int flags);

    bool isNeedGLES2() const;

    void *createGLES2Context(int flags);
};


#endif
