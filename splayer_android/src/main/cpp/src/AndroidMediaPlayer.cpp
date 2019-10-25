#include "AndroidMediaPlayer.h"

int AndroidMediaPlayer::setVideoSurface(ANativeWindow *nativeWindow) {
    if (DEBUG) {
        ALOGD(TAG, "%s nativeWindow = %p", __func__, nativeWindow);
    }
    if (!nativeWindow) {
        return ERROR;
    }
    if (videoDevice) {
        AndroidVideoDevice *device = dynamic_cast<AndroidVideoDevice *>(videoDevice);
        if (device) {
            device->setNativeWindow(nativeWindow);
        }
    }
    return SUCCESS;
}
