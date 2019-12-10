#include "splayer_ios_birdge.hpp"
#include "iOSMediaPlayer.h"
#include "Log.h"

const char *TAG = "[MP][BIRDGE][Main]";

bool IOS_DEBUG = false;

void setMediaPlayer(BirdgeContext *context, MediaPlayer *mp) {
    if (context != nullptr && mp != nullptr) {
        *context = (long long) mp;
    }
}

MediaPlayer *getMediaPlayer(BirdgeContext *context) {
    if (context != nullptr) {
        return (MediaPlayer *) (*context);
    }
    return nullptr;
}

void (^SwiftFunc)(void) = nullptr;

void CFuncTest(void) {
    SwiftFunc();
}

void _native_init(BirdgeContext *context) {
}

long _getRotate(BirdgeContext *context) {
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return 0;
    }
    return mp->getRotate();
}

long _getVideoWidth(BirdgeContext *context) {
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return 0;
    }
    return mp->getVideoWidth();
}

long _getVideoHeight(BirdgeContext *context) {
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return 0;
    }
    return mp->getVideoHeight();
}

bool _isPlaying(BirdgeContext *context) {
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return false;
    }
    return mp->isPlaying();
}

long _getDuration(BirdgeContext *context) {
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return 0L;
    }
    return mp->getDuration();
}

long _getCurrentPosition(BirdgeContext *context) {
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return 0L;
    }
    return mp->getCurrentPosition();
}

void _create(BirdgeContext *context) {

    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }

    MediaPlayer *mp = iOSMediaPlayer::Builder{}
            .withAudioDevice(nullptr)
            .withVideoDevice(nullptr)
            .withMediaSync(nullptr)
            .withMessageListener(nullptr)
            .withDebug(IOS_DEBUG)
            .build();

    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }

    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] media player = %p", __func__, mp);
    }

    int result = mp->create();

    if (result < 0) {
        // jniThrowException(env, "java/lang/IllegalStateException");
        ALOGE(TAG, "[%s] illegal state exception result=%d", __func__, result);
        return;
    }

    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] media player created", __func__);
    }

    setMediaPlayer(context, mp);
}

void _destroy(BirdgeContext *context) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp != nullptr) {
        mp->destroy();
        delete mp;
        setMediaPlayer(nullptr, nullptr);
    }
}

void _start(BirdgeContext *context) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->start();
}

void _play(BirdgeContext *context) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->play();
}

void _pause(BirdgeContext *context) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->pause();
}

void _stop(BirdgeContext *context) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->stop();
}

void _reset(BirdgeContext *context) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->stop();
}

void _seekTo(BirdgeContext *context, float timeMs) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] timeMs=%ld", __func__, timeMs);
    }
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->seekTo(timeMs);
}

void _setVolume(BirdgeContext *context, float leftVolume, float rightVolume) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] leftVolume=%lf rightVolume=%lf", __func__, leftVolume, rightVolume);
    }
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->setVolume(leftVolume, rightVolume);
}

void _setMute(BirdgeContext *context, bool mute) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] mute=%d", __func__, mute);
    }
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->setMute(mute);
}

void _setRate(BirdgeContext *context, float rate) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] speed=%lf", __func__, rate);
    }
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->setRate(rate);
}

void _setPitch(BirdgeContext *context, float pitch) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] pitch=%lf", __func__, pitch);
    }
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->setPitch(pitch);
}

void _setLooping(BirdgeContext *context, bool looping) {
    if (IOS_DEBUG) {
        ALOGD(TAG, "[%s] looping=%d", __func__, looping);
    }
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->setLooping(looping);
}

bool _isLooping(BirdgeContext *context) {
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
        return false;
    }
    return mp->isLooping() != 0;
}

void _setDataSource(BirdgeContext *context, const char *path) {
    _setDataSourceAndHeaders(context, path, nullptr, nullptr);
}

void _setDataSourceAndHeaders(BirdgeContext *context, const char *path, char *keys, void *values) {
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        // jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }

    if (path == nullptr) {
        // jniThrowException(env, "java/lang/IllegalArgumentException");
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
//    process_media_player_call(env, thiz, result, "java/io/IOException", "setDataSource failed.");
//    env->ReleaseStringUTFChars(path_, path);
}

void _setOptionS(BirdgeContext *context, long category, const char *type, const char *option) {
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
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

void _setOptionL(BirdgeContext *context, long category, const char *type, long option) {
    MediaPlayer *mp = getMediaPlayer(context);
    if (mp == nullptr) {
        ALOGE(TAG, "[%s] mp=%p", __func__, mp);
        // jniThrowException(env, "java/lang/IllegalStateException");
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

void _setSurface(BirdgeContext *context) {

}
