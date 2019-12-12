#include "splayer_ios_birdge.hpp"
#include "Log.h"
#include "iOSMediaPlayer.h"
#include "iOSMediaSync.h"
#include "iOSAudioDevice.h"
#include "iOSVideoDevice.h"

const char *TAG = "[MP][BIRDGE][Main]";

bool IOS_DEBUG = false;


class MessageListener : public IMessageListener {

private:
    _NMPReference *nmpReference = nullptr;
    _SMPReference *smpReference = nullptr;

public:
    _PostFromNativeReference postReference = nullptr;
public:

    MessageListener(_NMPReference *nmpReference, void *smpReference, _PostFromNativeReference func) {
        this->nmpReference = nmpReference;
        this->smpReference = smpReference;
        this->postReference = func;
    }

    ~MessageListener() {
        nmpReference = nullptr;
        smpReference = nullptr;
    }

    void onNotify(int msg, int ext1, int ext2, void *obj) {
        if (postReference != nullptr) {
            (postReference)(smpReference, msg, ext1, ext2);
        }
    }

    void onMessage(Msg *msg) override {

        if (IOS_DEBUG) {
            ALOGD(TAG, "[%s] what=%s arg1=%d", __func__, Msg::getMsgSimpleName(msg->what),
                    msg->arg1I);
        }

        switch (msg->what) {
            case Msg::MSG_FLUSH:
            case Msg::MSG_ERROR:
            case Msg::MSG_PLAY_STARTED:
            case Msg::MSG_PLAY_COMPLETED:
            case Msg::MSG_OPEN_INPUT:
            case Msg::MSG_STREAM_INFO:
            case Msg::MSG_VIDEO_SIZE_CHANGED:
            case Msg::MSG_SAR_CHANGED:
            case Msg::MSG_AUDIO_START:
            case Msg::MSG_AUDIO_RENDERING_START:
            case Msg::MSG_VIDEO_ROTATION_CHANGED:
            case Msg::MSG_SEEK_START:
            case Msg::MSG_SEEK_COMPLETE:
                onNotify(msg->what, msg->arg1I, msg->arg2I, nullptr);
                break;
        }
    }
};


static void setMediaPlayer(_NMPReference *mediaPlayerReference, MediaPlayer *mediaPlayer) {
    if (mediaPlayerReference != nullptr && mediaPlayer != nullptr) {
        *mediaPlayerReference = (long long) mediaPlayer;
    }
}

static MediaPlayer *getMediaPlayer(_NMPReference *mediaPlayerReference) {
    if (mediaPlayerReference != nullptr) {
        return (MediaPlayer *) (*mediaPlayerReference);
    }
    return nullptr;
}

void (*_postFromNative)(_SMPReference *smpReference, int msg, int ext1, int ext2) = nullptr;

void (*_throwException)(const char *_Nullable className) = nullptr;

void (*_throwExceptionWithNameAndMessage)(const char *_Nullable className, const char *_Nullable message) = nullptr;

void _native_init(_NMPReference *nmpReference) {
}

long _getRotate(_NMPReference *nmpReference) {
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return 0;
    }
    return mp->getRotate();
}

long _getVideoWidth(_NMPReference *nmpReference) {
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return 0;
    }
    return mp->getVideoWidth();
}

long _getVideoHeight(_NMPReference *nmpReference) {
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return 0;
    }
    return mp->getVideoHeight();
}

bool _isPlaying(_NMPReference *nmpReference) {
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return false;
    }
    return mp->isPlaying();
}

long _getDuration(_NMPReference *nmpReference) {
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return 0L;
    }
    return mp->getDuration();
}

long _getCurrentPosition(_NMPReference *nmpReference) {
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return 0L;
    }
    return mp->getCurrentPosition();
}

void _create(_NMPReference *nmpReference, _SMPReference *smpReference) {

    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }

    MediaPlayer *mp = iOSMediaPlayer::Builder{}
            .withAudioDevice(new iOSAudioDevice())
            .withVideoDevice(new iOSVideoDevice())
            .withMediaSync(new iOSMediaSync())
            .withMessageListener(new MessageListener(nmpReference, smpReference, _postFromNative))
            .withDebug(IOS_DEBUG)
            .build();

    _postFromNative = nullptr;

    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return;
    }

    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] media player = %p", __func__, mp);
    }

    int result = mp->create();

    if (result < 0) {
        _throwException("java/lang/IllegalStateException");
        ALOGE(TAG, "[%s] illegal state exception result=%d", __func__, result);
        return;
    }

    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] media player created", __func__);
    }

    setMediaPlayer(nmpReference, mp);
}

void _destroy(_NMPReference *nmpReference) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    _postFromNative = nullptr;
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp != nullptr) {
        mp->destroy();
        delete mp;
        setMediaPlayer(nullptr, nullptr);
    }
}

void _start(_NMPReference *nmpReference) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return;
    }
    mp->start();
}

void _play(_NMPReference *nmpReference) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return;
    }
    mp->play();
}

void _pause(_NMPReference *nmpReference) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return;
    }
    mp->pause();
}

void _stop(_NMPReference *nmpReference) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return;
    }
    mp->stop();
}

void _reset(_NMPReference *nmpReference) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return;
    }
    mp->stop();
}

void _seekTo(_NMPReference *nmpReference, float timeMs) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] timeMs=%ld", __func__, timeMs);
    }
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return;
    }
    mp->seekTo(timeMs);
}

void _setVolume(_NMPReference *nmpReference, float leftVolume, float rightVolume) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] leftVolume=%lf rightVolume=%lf", __func__, leftVolume, rightVolume);
    }
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return;
    }
    mp->setVolume(leftVolume, rightVolume);
}

void _setMute(_NMPReference *nmpReference, bool mute) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] mute=%d", __func__, mute);
    }
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return;
    }
    mp->setMute(mute);
}

void _setRate(_NMPReference *nmpReference, float rate) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] speed=%lf", __func__, rate);
    }
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return;
    }
    mp->setRate(rate);
}

void _setPitch(_NMPReference *nmpReference, float pitch) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] pitch=%lf", __func__, pitch);
    }
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return;
    }
    mp->setPitch(pitch);
}

void _setLooping(_NMPReference *nmpReference, bool looping) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] looping=%d", __func__, looping);
    }
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return;
    }
    mp->setLooping(looping);
}

bool _isLooping(_NMPReference *nmpReference) {
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return false;
    }
    return mp->isLooping() != 0;
}

void _setDataSource(_NMPReference *nmpReference, const char *path) {
    _setDataSourceAndHeaders(nmpReference, path, nullptr, nullptr);
}

// If exception is nullptr and opStatus is not OK, this method sends an error
// event to the client application; otherwise, if exception is not nullptr and
// opStatus is not OK, this method throws the given exception to the client
// application.
static void process_media_player_call(_NMPReference *nmpReference, int opStatus,
        const char *exception, const char *message) {
    if (exception == nullptr) {  // Don't throw exception. Instead, send an event.
        if (opStatus != SUCCESS) {
            MediaPlayer *mp = getMediaPlayer(nmpReference);
            if (mp != nullptr) {
                mp->notifyMsg(Msg::MSG_ERROR, opStatus);
            }
        }
    } else {  // Throw exception!
        if (opStatus == ERROR_INVALID_OPERATION) {
            _throwException("java/lang/IllegalStateException");
        } else if (opStatus == ERROR_PERMISSION_DENIED) {
            _throwException("java/lang/SecurityException");
        } else if (opStatus != SUCCESS) {
            if (strlen(message) > 230) {
                // if the message is too long, don't bother displaying the status code
                _throwExceptionWithNameAndMessage(exception, message);
            } else {
                char msg[256];
                // append the status code to the message
                sprintf(msg, "%s: status=0x%X", message, opStatus);
                _throwExceptionWithNameAndMessage(exception, msg);
            }
        }
    }
}

void _setDataSourceAndHeaders(_NMPReference *nmpReference, const char *path, char *keys, void *values) {
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        _throwException("java/lang/IllegalStateException");
        return;
    }

    if (path == nullptr) {
        _throwException("java/lang/IllegalArgumentException");
        return;
    }
    //
    //    const char *path = env->GetStringUTFChars(path_, 0);
    //    if (path == nullptr) {
    //        return;
    //    }
    //
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] path = %s", __func__, path);
    }

    const char *restrict = strstr(path, "mms://");
    char *restrict_to = restrict ? strdup(restrict) : nullptr;
    if (restrict_to != nullptr) {
        strncpy(restrict_to, "mmsh://", 6);
        puts(path);
    }
    //
    //    char *headers = nullptr;
    //    if (keys && values != nullptr) {
    //        int keysCount = env->GetArrayLength(keys);
    //        int valuesCount = env->GetArrayLength(values);
    //
    //        if (keysCount != valuesCount) {
    //            if (JNI_DEBUG) {
    //                ALOGE(TAG, "[%s] keys and values arrays have different length", __func__);
    //            }
    //            jniThrowException(env, "java/lang/IllegalArgumentException");
    //            return;
    //        }
    //
    //        int i = 0;
    //        const char *rawString = nullptr;
    //        char hdrs[2048];
    //
    //        for (i = 0; i < keysCount; i++) {
    //            jstring key = (jstring) env->GetObjectArrayElement(keys, i);
    //            rawString = env->GetStringUTFChars(key, nullptr);
    //            strcat(hdrs, rawString);
    //            strcat(hdrs, ": ");
    //            env->ReleaseStringUTFChars(key, rawString);
    //
    //            jstring value = (jstring) env->GetObjectArrayElement(values, i);
    //            rawString = env->GetStringUTFChars(value, nullptr);
    //            strcat(hdrs, rawString);
    //            strcat(hdrs, "\r\n");
    //            env->ReleaseStringUTFChars(value, rawString);
    //        }
    //
    //        headers = &hdrs[0];
    //    }
    //
    int result = mp->setDataSource(path, 0, nullptr);
    process_media_player_call(nmpReference, result, "java/io/IOException", "setDataSource failed.");
    //    env->ReleaseStringUTFChars(path_, path);
}

void _setOptionS(_NMPReference *nmpReference, long category, const char *type, const char *option) {
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return;
    }
    //    const char *type = env->GetStringUTFChars(type_, 0);
    //    const char *option = env->GetStringUTFChars(option_, 0);
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] type=%s option=%s", __func__, type, option);
    }
    if (type == nullptr || option == nullptr) {
        return;
    }
    mp->setOption(category, type, option);
}

void _setOptionL(_NMPReference *nmpReference, long category, const char *type, long option) {
    MediaPlayer *mp = getMediaPlayer(nmpReference);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        _throwException("java/lang/IllegalStateException");
        return;
    }
    //    const char *type = env->GetStringUTFChars(type_, 0);
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] type=%s option=%d", __func__, type, option);
    }
    if (type == nullptr) {
        return;
    }
    mp->setOption(category, type, option);
}

void _setSurface(_NMPReference *nmpReference) {

}
