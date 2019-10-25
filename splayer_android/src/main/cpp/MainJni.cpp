#include <jni.h>
#include <cassert>
#include <common/Log.h>
#include <AndroidJniHelp.h>
#include <android/native_window_jni.h>
#include <AndroidMediaPlayer.h>
#include <AndroidMediaSync.h>
#include <AndroidAudioDevice.h>
#include <utils/Log.h>

extern "C" {
#include <libavcodec/jni.h>
}

struct fields_t {
    jfieldID context;
    jmethodID post_event;
};
static fields_t fields;

static bool JNI_DEBUG = false;

static JavaVM *javaVM = nullptr;

static JNIEnv *getJNIEnv() {
    JNIEnv *env;
    assert(javaVM != nullptr);
    if (javaVM->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return nullptr;
    }
    return env;
}

const char *CLASS_NAME = "com/bzh/splayer/MediaPlayer";
const char *TAG = "MainJni";

class MessageListener : public IMessageListener {
    const char *const TAG = "MessageListener";

private:
    jclass mClass;
    jobject mObject;

public:

    MessageListener(JNIEnv *env, jobject thiz, jobject weak_thiz) {
        // Hold onto the MediaPlayer class for use in calling the static method
        // that posts events to the application thread.
        jclass clazz = env->GetObjectClass(thiz);
        if (clazz == nullptr) {
            ALOGE(TAG, "Can't find com/bzh/splayer/MediaPlayer");
            jniThrowException(env, "java/lang/Exception");
            return;
        }
        mClass = (jclass) env->NewGlobalRef(clazz);
        // We use a weak reference so the MediaPlayer object can be garbage collected.
        // The reference is only used as a proxy for callbacks.
        mObject = env->NewGlobalRef(weak_thiz);
    }

    ~MessageListener() {
        JNIEnv *env = getJNIEnv();
        env->DeleteGlobalRef(mObject);
        env->DeleteGlobalRef(mClass);
    }

    void notify(int msg, int ext1, int ext2, void *obj) {
        JNIEnv *env = getJNIEnv();

        bool status = (javaVM->AttachCurrentThread(&env, nullptr) >= 0);

        env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, obj);

        if (env->ExceptionCheck()) {
            if (DEBUG) {
                ALOGW(TAG, "An exception occurred while notifying an event.");
            }
            env->ExceptionClear();
        }

        if (status) {
            javaVM->DetachCurrentThread();
        }
    }

    void onMessage(Msg *msg) override {
        if (DEBUG) {
            ALOGD(TAG, "%s what = %s arg1 = %d arg2 = %d", __func__,
                  Msg::getMsgSimpleName(msg->what),
                  msg->arg1,
                  msg->arg2);
        }
//        if (mediaPlayer) {
//            switch (msg->what) {
//                case Msg::MSG_REQUEST_PLAY_OR_PAUSE:
//                    if (mediaPlayer->isPlaying()) {
//                        mediaPlayer->pause();
//                    } else {
//                        mediaPlayer->play();
//                    }
//                    break;
//                case Msg::MSG_REQUEST_DESTROY:
//                    if (mediaPlayer) {
//                        mediaPlayer->destroy();
//                    }
//                    break;
//                case Msg::MSG_REQUEST_START:
//                    break;
//                case Msg::MSG_REQUEST_SEEK:
//                    int increment = msg->arg1;
//                    if (mediaPlayer) {
//                        mediaPlayer->seekTo(increment);
//                    }
//                    break;
//            }
//        } else {
//            if (DEBUG) {
//                ALOGE(TAG, "%s media player is null", __func__);
//            }
//        }
    }
};


static MediaPlayer *getMediaPlayer(JNIEnv *env, jobject thiz) {
    MediaPlayer *mp = (MediaPlayer *) env->GetLongField(thiz, fields.context);
    return mp;
}

static MediaPlayer *setMediaPlayer(JNIEnv *env, jobject thiz, long mediaPlayer) {
    MediaPlayer *old = (MediaPlayer *) env->GetLongField(thiz, fields.context);
    env->SetLongField(thiz, fields.context, mediaPlayer);
    return old;
}

// If exception is nullptr and opStatus is not OK, this method sends an error
// event to the client application; otherwise, if exception is not nullptr and
// opStatus is not OK, this method throws the given exception to the client
// application.
static void process_media_player_call(JNIEnv *env, jobject thiz, int opStatus,
                                      const char *exception, const char *message) {
    if (exception == nullptr) {  // Don't throw exception. Instead, send an event.
        if (opStatus != SUCCESS) {
            MediaPlayer *mp = getMediaPlayer(env, thiz);
            if (mp != 0) {
                // mp->notify(MEDIA_ERROR, opStatus, 0);
            }
        }
    } else {  // Throw exception!
//        if (opStatus == (int) INVALID_OPERATION) {
//            jniThrowException(env, "java/lang/IllegalStateException");
//        } else if (opStatus == (int) PERMISSION_DENIED) {
//            jniThrowException(env, "java/lang/SecurityException");
//        } else if (opStatus != (int) OK) {
//            if (strlen(message) > 230) {
//                // if the message is too long, don't bother displaying the status code
//                jniThrowException(env, exception, message);
//            } else {
//                char msg[256];
//                // append the status code to the message
//                sprintf(msg, "%s: status=0x%X", message, opStatus);
//                jniThrowException(env, exception, msg);
//            }
//        }
    }
}

void MediaPlayer_setDataSourceAndHeaders(JNIEnv *env, jobject thiz, jstring path_,
                                         jobjectArray keys, jobjectArray values) {

    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }

    if (path_ == nullptr) {
        jniThrowException(env, "java/lang/IllegalArgumentException");
        return;
    }

    const char *path = env->GetStringUTFChars(path_, 0);
    if (path == nullptr) {
        return;
    }

    if (JNI_DEBUG) {
        ALOGD(TAG, "%s path = %s", __func__, path);
    }

    const char *restrict = strstr(path, "mms://");
    char *restrict_to = restrict ? strdup(restrict) : nullptr;
    if (restrict_to != nullptr) {
        strncpy(restrict_to, "mmsh://", 6);
        puts(path);
    }

    char *headers = nullptr;
    if (keys && values != nullptr) {
        int keysCount = env->GetArrayLength(keys);
        int valuesCount = env->GetArrayLength(values);

        if (keysCount != valuesCount) {
            if (DEBUG) {
                ALOGE(TAG, "keys and values arrays have different length");
            }
            jniThrowException(env, "java/lang/IllegalArgumentException");
            return;
        }

        int i = 0;
        const char *rawString = nullptr;
        char hdrs[2048];

        for (i = 0; i < keysCount; i++) {
            jstring key = (jstring) env->GetObjectArrayElement(keys, i);
            rawString = env->GetStringUTFChars(key, nullptr);
            strcat(hdrs, rawString);
            strcat(hdrs, ": ");
            env->ReleaseStringUTFChars(key, rawString);

            jstring value = (jstring) env->GetObjectArrayElement(values, i);
            rawString = env->GetStringUTFChars(value, nullptr);
            strcat(hdrs, rawString);
            strcat(hdrs, "\r\n");
            env->ReleaseStringUTFChars(value, rawString);
        }

        headers = &hdrs[0];
    }

    int result = mp->setDataSource(path, 0, headers);
    process_media_player_call(env, thiz, result, "java/io/IOException", "setDataSource failed.");
    env->ReleaseStringUTFChars(path_, path);
}

void MediaPlayer_setDataSource(JNIEnv *env, jobject thiz, jstring path_) {
    MediaPlayer_setDataSourceAndHeaders(env, thiz, path_, nullptr, nullptr);
}

void MediaPlayer_setDataSourceFD(JNIEnv *env, jobject thiz, jobject fileDescriptor,
                                 jlong offset, jlong length) {
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }

    if (fileDescriptor == nullptr) {
        jniThrowException(env, "java/lang/IllegalArgumentException");
        return;
    }

    int fd = jniGetFDFromFileDescriptor(env, fileDescriptor);
    if (offset < 0 || length < 0 || fd < 0) {
        if (offset < 0) {
            if (DEBUG) {
                ALOGE(TAG, "negative offset (%lld)", offset);
            }
        }
        if (length < 0) {
            if (DEBUG) {
                ALOGE(TAG, "negative length (%lld)", length);
            }
        }
        if (fd < 0) {
            if (DEBUG) {
                ALOGE(TAG, "invalid file descriptor");
            }
        }
        jniThrowException(env, "java/lang/IllegalArgumentException");
        return;
    }

    char path[256] = "";
    int myfd = dup(fd);
    char str[20];
    sprintf(str, "pipe:%d", myfd);
    strcat(path, str);

    int result = mp->setDataSource(path, offset, nullptr);
    process_media_player_call(env, thiz, result, "java/io/IOException",
                              "setDataSourceFD failed.");

}

void MediaPlayer_init(JNIEnv *env) {

    jclass clazz = env->FindClass(CLASS_NAME);
    if (clazz == nullptr) {
        ALOGE(TAG, "not find class, class=%s", CLASS_NAME);
        return;
    }

    // 获取DEBUG
    JNI_DEBUG = env->GetStaticBooleanField(clazz, env->GetStaticFieldID(clazz, "DEBUG", "Z"));

    RENDERER_DEBUG = JNI_DEBUG;

    if (JNI_DEBUG) {
        ALOGD(TAG, __func__);
    }

    // 获取Context Id
    fields.context = env->GetFieldID(clazz, "mNativeContext", "J");
    if (fields.context == nullptr) {
        ALOGE(TAG, "not find field mNativeContext");
        return;
    }

    if (JNI_DEBUG) {
        ALOGD(TAG, "%s context = %p", __func__, fields.context);
    }

    // 获取PostEvent Id
    fields.post_event = env->GetStaticMethodID(clazz,
                                               "postEventFromNative",
                                               "(Ljava/lang/Object;IIILjava/lang/Object;)V");
    if (fields.post_event == nullptr) {
        ALOGE(TAG, "not find static method postEventFromNative");
        return;
    }

    if (JNI_DEBUG) {
        ALOGD(TAG, "%s post_event = %p", __func__, fields.post_event);
    }

    env->DeleteLocalRef(clazz);
}

void MediaPlayer_create(JNIEnv *env, jobject thiz, jobject mediaplayer_this) {

    if (JNI_DEBUG) {
        ALOGD(TAG, __func__);
    }

    MediaPlayer *mp = AndroidMediaPlayer::Builder{}
            .withAudioDevice(new AndroidAudioDevice())
            .withVideoDevice(new AndroidVideoDevice())
            .withMediaSync(new AndroidMediaSync())
            .withMessageListener(new MessageListener(env, thiz, mediaplayer_this))
            .withDebug(JNI_DEBUG)
            .build();

    if (mp == nullptr) {
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return;
    }

    if (JNI_DEBUG) {
        ALOGD(TAG, "%s media player = %p", __func__, mp);
    }

    int result = mp->create();

    if (result < 0) {
        jniThrowException(env, "java/lang/IllegalStateException");
    }

    if (JNI_DEBUG) {
        ALOGD(TAG, "%s media player created", __func__);
    }

    // Stow our new C++ MediaPlayer in an opaque field in the Java object.
    setMediaPlayer(env, thiz, (long) mp);
}

void MediaPlayer_destroy(JNIEnv *env, jobject thiz) {
    if (JNI_DEBUG) {
        ALOGD(TAG, __func__);
    }
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp != nullptr) {
        mp->destroy();
        delete mp;
        setMediaPlayer(env, thiz, 0);
    }
}

void MediaPlayer_reset(JNIEnv *env, jobject thiz) {
    if (JNI_DEBUG) {
        ALOGD(TAG, __func__);
    }
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->stop();
}

void MediaPlayer_finalize(JNIEnv *env, jobject thiz) {
    if (JNI_DEBUG) {
        ALOGD(TAG, __func__);
    }
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp != nullptr) {
        ALOGE(TAG, "%s MediaPlayer finalized without being released", __func__);
    }
    MediaPlayer_destroy(env, thiz);
}

void MediaPlayer_setVideoSurface(JNIEnv *env, jobject thiz, jobject surface) {
    if (JNI_DEBUG) {
        ALOGD(TAG, "%s surface=%p", __func__, surface);
    }
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    ANativeWindow *window = nullptr;
    if (surface != nullptr) {
        window = ANativeWindow_fromSurface(env, surface);
    }
    ((AndroidMediaPlayer *) mp)->setVideoSurface(window);
}

void MediaPlayer_setLooping(JNIEnv *env, jobject thiz, jboolean looping) {
    if (JNI_DEBUG) {
        ALOGD(TAG, "%s looping=%d", __func__, looping);
    }
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->setLooping(looping);
}

jboolean MediaPlayer_isLooping(JNIEnv *env, jobject thiz) {
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return JNI_FALSE;
    }
    return (jboolean) (mp->isLooping() ? JNI_TRUE : JNI_FALSE);
}

void MediaPlayer_prepare(JNIEnv *env, jobject thiz) {
    if (JNI_DEBUG) {
        ALOGD(TAG, __func__);
    }
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->start();
}

void MediaPlayer_prepareAsync(JNIEnv *env, jobject thiz) {
    if (JNI_DEBUG) {
        ALOGD(TAG, __func__);
    }
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->start();
}

void MediaPlayer_start(JNIEnv *env, jobject thiz) {
    if (JNI_DEBUG) {
        ALOGD(TAG, __func__);
    }
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->start();
}

void MediaPlayer_pause(JNIEnv *env, jobject thiz) {
    if (JNI_DEBUG) {
        ALOGD(TAG, __func__);
    }
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->pause();
}

void MediaPlayer_play(JNIEnv *env, jobject thiz) {
    if (JNI_DEBUG) {
        ALOGD(TAG, __func__);
    }
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->play();
}

void MediaPlayer_stop(JNIEnv *env, jobject thiz) {
    if (JNI_DEBUG) {
        ALOGD(TAG, __func__);
    }
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->stop();
}

void MediaPlayer_seekTo(JNIEnv *env, jobject thiz, jfloat timeMs) {
    if (JNI_DEBUG) {
        ALOGD(TAG, "%s timeMs=%ld", __func__, timeMs);
    }
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->seekTo(timeMs);
}

void MediaPlayer_setMute(JNIEnv *env, jobject thiz, jboolean mute) {
    if (JNI_DEBUG) {
        ALOGD(TAG, "%s mute=%d", __func__, mute);
    }
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->setMute(mute);
}

void MediaPlayer_setVolume(JNIEnv *env, jobject thiz, jfloat leftVolume, jfloat rightVolume) {
    if (JNI_DEBUG) {
        ALOGD(TAG, "%s leftVolume=%lf rightVolume=%lf", __func__, leftVolume, rightVolume);
    }
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->setVolume(leftVolume, rightVolume);
}

void MediaPlayer_setRate(JNIEnv *env, jobject thiz, jfloat speed) {
    if (JNI_DEBUG) {
        ALOGD(TAG, "%s speed=%lf", __func__, speed);
    }
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->setRate(speed);
}

void MediaPlayer_setPitch(JNIEnv *env, jobject thiz, jfloat pitch) {
    if (JNI_DEBUG) {
        ALOGD(TAG, "%s pitch=%lf", __func__, pitch);
    }
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    mp->setPitch(pitch);
}

jlong MediaPlayer_getCurrentPosition(JNIEnv *env, jobject thiz) {
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return 0L;
    }
    return mp->getCurrentPosition();
}

jlong MediaPlayer_getDuration(JNIEnv *env, jobject thiz) {
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return 0L;
    }
    return mp->getDuration();
}

jboolean MediaPlayer_isPlaying(JNIEnv *env, jobject thiz) {
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return JNI_FALSE;
    }
    return (jboolean) (mp->isPlaying() ? JNI_TRUE : JNI_FALSE);
}

jint MediaPlayer_getRotate(JNIEnv *env, jobject thiz) {
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return 0;
    }
    return mp->getRotate();
}

jint MediaPlayer_getVideoWidth(JNIEnv *env, jobject thiz) {
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return 0;
    }
    return mp->getVideoWidth();
}

jint MediaPlayer_getVideoHeight(JNIEnv *env, jobject thiz) {
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return 0;
    }
    return mp->getVideoHeight();
}

void MediaPlayer_setOption(JNIEnv *env,
                           jobject thiz,
                           int category,
                           jstring type_, jstring option_
) {
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    const char *type = env->GetStringUTFChars(type_, 0);
    const char *option = env->GetStringUTFChars(option_, 0);
    if (JNI_DEBUG) {
        ALOGD(TAG, "%s type=%s option=%s", __func__, type, option);
    }
    if (type == nullptr || option == nullptr) {
        return;
    }
    mp->setOption(category, type, option);
    env->ReleaseStringUTFChars(type_, type);
    env->ReleaseStringUTFChars(option_, option);
}

void MediaPlayer_setOptionLong(JNIEnv *env,
                               jobject thiz,
                               int category,
                               jstring type_,
                               jlong option_
) {
    MediaPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == nullptr) {
        jniThrowException(env, "java/lang/IllegalStateException");
        return;
    }
    const char *type = env->GetStringUTFChars(type_, 0);
    if (JNI_DEBUG) {
        ALOGD(TAG, "%s type=%s option=%d", __func__, type, option_);
    }
    if (type == nullptr) {
        return;
    }
    mp->setOption(category, type, option_);
    env->ReleaseStringUTFChars(type_, type);
}


static const JNINativeMethod gMethods[] = {
        {"_setDataSource",      "(Ljava/lang/String;)V",                                       (void *) MediaPlayer_setDataSource},
        {"_setDataSource",      "(Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;)V", (void *) MediaPlayer_setDataSourceAndHeaders},
        {"_setDataSource",      "(Ljava/io/FileDescriptor;JJ)V",                               (void *) MediaPlayer_setDataSourceFD},
        {"_setVideoSurface",    "(Landroid/view/Surface;)V",                                   (void *) MediaPlayer_setVideoSurface},
        {"_prepare",            "()V",                                                         (void *) MediaPlayer_prepare},
        {"_prepareAsync",       "()V",                                                         (void *) MediaPlayer_prepareAsync},
        {"_start",              "()V",                                                         (void *) MediaPlayer_start},
        {"_stop",               "()V",                                                         (void *) MediaPlayer_stop},
        {"_resume",             "()V",                                                         (void *) MediaPlayer_play},
        {"_getRotate",          "()I",                                                         (void *) MediaPlayer_getRotate},
        {"_getVideoWidth",      "()I",                                                         (void *) MediaPlayer_getVideoWidth},
        {"_getVideoHeight",     "()I",                                                         (void *) MediaPlayer_getVideoHeight},
        {"_seekTo",             "(F)V",                                                        (void *) MediaPlayer_seekTo},
        {"_pause",              "()V",                                                         (void *) MediaPlayer_pause},
        {"_isPlaying",          "()Z",                                                         (void *) MediaPlayer_isPlaying},
        {"_getCurrentPosition", "()J",                                                         (void *) MediaPlayer_getCurrentPosition},
        {"_getDuration",        "()J",                                                         (void *) MediaPlayer_getDuration},
        {"_release",            "()V",                                                         (void *) MediaPlayer_destroy},
        {"_reset",              "()V",                                                         (void *) MediaPlayer_reset},
        {"_setLooping",         "(Z)V",                                                        (void *) MediaPlayer_setLooping},
        {"_isLooping",          "()Z",                                                         (void *) MediaPlayer_isLooping},
        {"_setVolume",          "(FF)V",                                                       (void *) MediaPlayer_setVolume},
        {"_setMute",            "(Z)V",                                                        (void *) MediaPlayer_setMute},
        {"_setRate",            "(F)V",                                                        (void *) MediaPlayer_setRate},
        {"_setPitch",           "(F)V",                                                        (void *) MediaPlayer_setPitch},
        {"_native_init",        "()V",                                                         (void *) MediaPlayer_init},
        {"_native_setup",       "(Ljava/lang/Object;)V",                                       (void *) MediaPlayer_create},
        {"_native_finalize",    "()V",                                                         (void *) MediaPlayer_finalize},
        {"_setOption",          "(ILjava/lang/String;Ljava/lang/String;)V",                    (void *) MediaPlayer_setOption},
        {"_setOption",          "(ILjava/lang/String;J)V",                                     (void *) MediaPlayer_setOptionLong}
};

// 注册Native方法
static int registerMediaPlayerMethod(JNIEnv *env) {
    int numMethods = (sizeof(gMethods) / sizeof((gMethods)[0]));
    jclass clazz = env->FindClass(CLASS_NAME);
    if (clazz == nullptr) {
        ALOGE(TAG, "Native registration unable to find class '%s'", CLASS_NAME);
        return JNI_ERR;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        ALOGE(TAG, "Native registration unable to find class '%s'", CLASS_NAME);
        return JNI_ERR;
    }
    env->DeleteLocalRef(clazz);
    return JNI_OK;
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    av_jni_set_java_vm(vm, nullptr);
    javaVM = vm;
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    if (registerMediaPlayerMethod(env) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_4;
}

