#include "EglHelper.h"

EglHelper::EglHelper() {
    eglDisplay = EGL_NO_DISPLAY;
    eglConfig = nullptr;
    eglContext = EGL_NO_CONTEXT;
    glVersion = -1;
    // 设置时间戳方法，只有Android存在这个方法
    eglPresentationTimeANDROID = nullptr;
}

EglHelper::~EglHelper() {
    release();
}

int EglHelper::init(int flags) {
    return init(EglContext::getInstance()->getContext(), flags);
}

int EglHelper::init(EGLContext sharedContext, int flags) {

    if (eglDisplay != EGL_NO_DISPLAY) {
        ALOGE(TAG, "[%s] EGL already set up", __func__);
        return -1;
    }

    if (sharedContext == nullptr) {
        ALOGE(TAG, "[%s] Shared Context is null", __func__);
        sharedContext = EGL_NO_CONTEXT;
    } else {
        if (ENGINE_DEBUG) {
            ALOGD(TAG, "[%s] Main EGLContext is created!", __func__);
        }
    }

    // 获取EGLDisplay
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDisplay == EGL_NO_DISPLAY) {
        // 无法打开到底层窗口系统的连接
        ALOGE(TAG, "[%s] unable to get EGLDisplay", __func__);
        return -1;
    }

    // 初始化mEGLDisplay
    if (!eglInitialize(eglDisplay, nullptr, nullptr)) {
        eglDisplay = EGL_NO_DISPLAY;
        ALOGE(TAG, "[%s] unable to initialize EGLDisplay", __func__);
        return -1;
    }

    // 判断是否尝试使用GLES3
    if ((flags & FLAG_TRY_GLES3) != 0) {
        EGLConfig config = getConfig(flags, 3);
        if (config != nullptr) {
            int attributeList[] = {
                    EGL_CONTEXT_CLIENT_VERSION, 3,
                    EGL_NONE
            };
            EGLContext context = eglCreateContext(eglDisplay, config, sharedContext, attributeList);
            checkEglError("eglCreateContext");
            if (eglGetError() == EGL_SUCCESS) {
                eglConfig = config;
                eglContext = context;
                glVersion = 3;
            }
        }
    }

    // 判断如果GLES3的EGLContext没有获取到，则尝试使用GLES2
    if (eglContext == EGL_NO_CONTEXT) {
        EGLConfig config = getConfig(flags, 2);
        int attributeList[] = {
                EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL_NONE
        };
        EGLContext context = eglCreateContext(eglDisplay, config, sharedContext, attributeList);
        checkEglError("eglCreateContext");
        if (eglGetError() == EGL_SUCCESS) {
            eglConfig = config;
            eglContext = context;
            glVersion = 2;
        }
    }

    // 获取eglPresentationTimeANDROID方法的地址
    eglPresentationTimeANDROID = (EGL_PRESENTATION_TIME_ANDROIDPROC) eglGetProcAddress(
            "eglPresentationTimeANDROID");

    if (!eglPresentationTimeANDROID) {
        if (ENGINE_DEBUG) {
            ALOGD(TAG, "[%s] eglPresentationTimeANDROID is not available!", __func__);
        }
    }

    int values[1] = {0};
    eglQueryContext(eglDisplay, eglContext, EGL_CONTEXT_CLIENT_VERSION, values);

    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s] EGLContext created, client version %d", __func__, values[0]);
    }

    return 1;
}

void EglHelper::release() {
    if (eglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
    if (eglContext != EGL_NO_CONTEXT) {
        eglDestroyContext(eglDisplay, eglContext);
    }
    if (eglDisplay != EGL_NO_DISPLAY) {
        eglReleaseThread();
        eglTerminate(eglDisplay);
    }
    eglDisplay = EGL_NO_DISPLAY;
    eglConfig = nullptr;
    eglContext = EGL_NO_CONTEXT;
}

EGLContext EglHelper::getEglContext() {
    return eglContext;
}

void EglHelper::destroySurface(EGLSurface eglSurface) {
    eglDestroySurface(eglDisplay, eglSurface);
}

EGLSurface EglHelper::createSurface(ANativeWindow *surface) {
    if (surface == nullptr) {
        ALOGE(TAG, "[%s] Window surface is NULL!", __func__);
        return nullptr;
    }
    int attributeList[] = {
            EGL_NONE
    };
    EGLSurface eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, surface, attributeList);
    checkEglError("eglCreateWindowSurface");
    if (eglSurface == EGL_NO_SURFACE) {
        ALOGE(TAG, "[%s] EGLSurface was null", __func__);
    }
    return eglSurface;
}

EGLSurface EglHelper::createSurface(int width, int height) {
    int attributeList[] = {
            EGL_WIDTH, width,
            EGL_HEIGHT, height,
            EGL_NONE
    };
    EGLSurface eglSurface = eglCreatePbufferSurface(eglDisplay, eglConfig, attributeList);
    checkEglError("eglCreatePbufferSurface");
    if (eglSurface == EGL_NO_SURFACE) {
        ALOGE(TAG, "[%s] EGLSurface was null", __func__);
    }
    return eglSurface;
}

void EglHelper::makeCurrent(EGLSurface eglSurface) {
    if (eglDisplay == EGL_NO_DISPLAY) {
        if (ENGINE_DEBUG) {
            ALOGD(TAG, "[%s] Note: makeCurrent w/o display.", __func__);
        }
    }
    if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
        ALOGE(TAG, "[%s] egl mark current error", __func__);
    }
}

void EglHelper::makeCurrent(EGLSurface drawSurface, EGLSurface readSurface) {
    if (eglDisplay == EGL_NO_DISPLAY) {
        if (ENGINE_DEBUG) {
            ALOGD(TAG, "[%s] Note: makeCurrent w/o display.", __func__);
        }
    }
    if (!eglMakeCurrent(eglDisplay, drawSurface, readSurface, eglContext)) {
        ALOGE(TAG, "[%s] egl mark current error", __func__);
    }
}

void EglHelper::makeNothingCurrent() {
    if (!eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
        ALOGE(TAG, "[%s] egl mark current error", __func__);
    }
}

int EglHelper::swapBuffers(EGLSurface eglSurface) {
    if (!eglSwapBuffers(eglDisplay, eglSurface)) {
        return eglGetError();
    }
    return EGL_SUCCESS;
}

void EglHelper::setPresentationTime(EGLSurface eglSurface, long nsecs) {
    if (eglPresentationTimeANDROID != nullptr) {
        eglPresentationTimeANDROID(eglDisplay, eglSurface, nsecs);
    }
}

bool EglHelper::isCurrent(EGLSurface eglSurface) {
    return (eglContext == eglGetCurrentContext())
           && (eglSurface == eglGetCurrentSurface(EGL_DRAW));
}

int EglHelper::querySurface(EGLSurface eglSurface, int what) {
    int value;
    eglQuerySurface(eglContext, eglSurface, what, &value);
    return value;
}

const char *EglHelper::queryString(int what) {
    return eglQueryString(eglDisplay, what);
}

int EglHelper::getGlVersion() {
    return glVersion;
}

void EglHelper::checkEglError(const char *msg) {
    int error;
    if ((error = eglGetError()) != EGL_SUCCESS) {
        ALOGE(TAG, "%s: EGL error: %x", msg, error);
    }
}

EGLConfig EglHelper::getConfig(int flags, int version) {
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
        if (ENGINE_DEBUG) {
            ALOGW(TAG, "[%s] unable to find RGB8888 / %d  EGLConfig", __func__, version);
        }
        return nullptr;
    }
    return configs;
}

int EglHelper::getSurfaceWidth(EGLSurface surface) {
    EGLint width = 0;
    if (!eglQuerySurface(eglDisplay, surface, EGL_WIDTH, &width)) {
        ALOGE(TAG, "[%s] [EGL] eglQuerySurface(EGL_WIDTH) returned error %d", __func__,
              eglGetError());
        return 0;
    }
    return width;
}

int EglHelper::getSurfaceHeight(EGLSurface surface) {
    EGLint height = 0;
    if (!eglQuerySurface(eglDisplay, surface, EGL_HEIGHT, &height)) {
        ALOGE(TAG, "[%s] [EGL] eglQuerySurface(EGL_HEIGHT) returned error %d", __func__,
              eglGetError());
        return 0;
    }
    return height;
}
