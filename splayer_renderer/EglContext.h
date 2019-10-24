#ifndef RENDERER_EGLCONTEXT_H
#define RENDERER_EGLCONTEXT_H

#include <mutex>
#include <android/native_window.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#include "EglHelper.h"
#include "Log.h"

/**
 * EGLContext 上下文，为了方便使用SharedContext而造的
 * https://woshijpf.github.io/android/2017/09/04/Android%E7%B3%BB%E7%BB%9F%E5%9B%BE%E5%BD%A2%E6%A0%88OpenGLES%E5%92%8CEGL%E4%BB%8B%E7%BB%8D.html
 * https://www.khronos.org/registry/EGL/sdk/docs/man/html/eglGetDisplay.xhtml
 */
class EglContext {

    const char *const TAG = "EglContext";

public:
    static EglContext *getInstance();

    void destroy();

    EGLContext getContext();

private:

    static EglContext *instance;
    static std::mutex mutex;

    EGLContext eglContext = nullptr;
    EGLDisplay eglDisplay = nullptr;

private:

    EglContext();

    virtual ~EglContext();

    int init(int flags);

    int release();

    EGLConfig getConfig(int flags, int version);

    void checkEglError(const char *msg);

};


#endif
