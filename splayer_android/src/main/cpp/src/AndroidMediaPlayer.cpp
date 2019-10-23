#include "AndroidMediaPlayer.h"

int AndroidMediaPlayer::setVideoSurface(ANativeWindow *nativeWindow) {
    if (DEBUG) {
        ALOGD(TAG, "%s nativeWindow=%p", __func__, nativeWindow);
    }
    if (!nativeWindow) {
        return ERROR;
    }
    if (videoDevice) {
        GLESVideoDevice *device = dynamic_cast<GLESVideoDevice *>(videoDevice);
        if (device) {
            device->onSurfaceCreated(nativeWindow);
        }
    }
    return SUCCESS;
}
