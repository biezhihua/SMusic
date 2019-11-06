#include "EglContext.h"

EglContext *EglContext::instance;

std::mutex  EglContext::mutex;

EglContext::EglContext() {
    eglDisplay = EGL_NO_DISPLAY;
    eglContext = EGL_NO_CONTEXT;
    init(FLAG_TRY_GLES3);
}

EglContext::~EglContext() {
    release();
}

EglContext *EglContext::getInstance() {
    if (!instance) {
        std::unique_lock<std::mutex> lock(mutex);
        if (!instance) {
            instance = new(std::nothrow) EglContext();
        }
    }
    return instance;
}

int EglContext::init(int flags) {

    if (eglDisplay != EGL_NO_DISPLAY) {
        ALOGE(TAG, "EGL already set up");
        return -1;
    }

    // 获取EGLDisplay
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDisplay == EGL_NO_DISPLAY) {
        ALOGE(TAG, "unable to get EGLDisplay");
        return -1;
    }

    // 初始化mEGLDisplay
    if (!eglInitialize(eglDisplay, nullptr, nullptr)) {
        eglDisplay = EGL_NO_DISPLAY;
        ALOGE(TAG, "unable to initialize EGLDisplay.");
        return -1;
    }

    // 判断是否尝试使用GLES3
    if (isGLES3(flags)) {
        eglContext = createGLES3Context(flags);
    }

    // 判断如果GLES3的EGLContext没有获取到，则尝试使用GLES2
    if (isNeedGLES2()) {
        eglContext = createGLES2Context(flags);
    }

    int values[1] = {0};
    eglQueryContext(eglDisplay, eglContext, EGL_CONTEXT_CLIENT_VERSION, values);
    if (RENDERER_DEBUG) {
        ALOGD(TAG, "EGLContext created, client version %d", values[0]);
    }
    return 1;
}

void *EglContext::createGLES2Context(int flags) {
    EGLConfig config = getConfig(flags, 2);
    int attributeList[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };
    EGLContext context = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, attributeList);
    checkEglError("eglCreateContext");
    if (eglGetError() == EGL_SUCCESS) {
        return context;
    }
    return nullptr;
}

bool EglContext::isNeedGLES2() const { return eglContext == EGL_NO_CONTEXT; }

EGLContext EglContext::createGLES3Context(int flags) {
    EGLConfig config = getConfig(flags, 3);
    if (config != nullptr) {
        int attributeList[] = {
                EGL_CONTEXT_CLIENT_VERSION, 3,
                EGL_NONE
        };
        EGLContext context = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT,
                                              attributeList);
        checkEglError("eglCreateContext");
        if (eglGetError() == EGL_SUCCESS) {
            return context;
        }
    }
    return nullptr;
}

bool EglContext::isGLES3(int flags) const { return (flags & FLAG_TRY_GLES3) != 0; }

/// https://www.khronos.org/registry/EGL/sdk/docs/man/html/eglChooseConfig.xhtml
EGLConfig EglContext::getConfig(int flags, int version) {
    int renderType = EGL_OPENGL_ES2_BIT;
    if (version >= 3) {
        renderType |= EGL_OPENGL_ES3_BIT_KHR;
    }
    int attributeList[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_RENDERABLE_TYPE, renderType,
            EGL_NONE, 0,      // placeholder for recordable [@-3]
            EGL_NONE
    };
    int length = sizeof(attributeList) / sizeof(attributeList[0]);
    if ((flags & FLAG_RECORDABLE) != 0) {
        attributeList[length - 3] = EGL_RECORDABLE_ANDROID;
        attributeList[length - 2] = 1;
    }
    EGLConfig configs = nullptr;
    int numConfigs;
    if (!eglChooseConfig(eglDisplay, attributeList, &configs, 1, &numConfigs)) {
        if (RENDERER_DEBUG) {
            ALOGW(TAG, "unable to find RGB8888 / %d  EGLConfig", version);
        }
        return nullptr;
    }
    return configs;
}

int EglContext::release() {
    if (eglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
    if (eglContext != EGL_NO_CONTEXT) {
        eglDestroyContext(eglDisplay, eglContext);
    }
    eglDisplay = EGL_NO_DISPLAY;
    eglContext = EGL_NO_CONTEXT;
    return 1;
}

void EglContext::destroy() {
    if (instance) {
        std::unique_lock<std::mutex> lock(mutex);
        if (!instance) {
            delete instance;
            instance = nullptr;
        }
    }
}

void EglContext::checkEglError(const char *msg) {
    int error;
    if ((error = eglGetError()) != EGL_SUCCESS) {
        ALOGE(TAG, "%s: EGL error: %x", msg, error);
    }
}

EGLContext EglContext::getContext() {
    return eglContext;
}

