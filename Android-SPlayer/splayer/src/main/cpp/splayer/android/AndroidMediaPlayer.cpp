#include "AndroidMediaPlayer.h"



AndroidMediaPlayer::AndroidMediaPlayer() {
    ALOGD(__func__);
}

AndroidMediaPlayer::~AndroidMediaPlayer() {
    ALOGD(__func__);
}

AOut *AndroidMediaPlayer::createAOut() {
    ALOGD(__func__);
    return new AndroidAOut();
}

VOut *AndroidMediaPlayer::createSurface() {
    ALOGD(__func__);
    return new AndroidVOutSurface();
}

Pipeline *AndroidMediaPlayer::createPipeline() {
    ALOGD(__func__);
    return new AndroidPipeline();
}

int AndroidMediaPlayer::messageLoop() {
    while (true) {
        Message msg;

        ALOGD("%s msg=%p", __func__, &msg);

        int ret = play->getMsg(&msg, true);
        if (ret == EXIT_FAILURE) {
            break;
        }

        switch (msg.what) {
            case Message::MSG_FLUSH:
                ALOGD("MSG_FLUSH:\n");
                postEvent(MEDIA_NOP, 0, 0);
                break;
            case Message::MSG_ERROR:
                ALOGD("MSG_ERROR: %d\n", msg.arg1);
                postEvent(MEDIA_ERROR, MEDIA_ERROR_IJK_PLAYER, msg.arg1);
                break;
            case Message::MSG_PREPARED:
                ALOGD("MSG_PREPARED:\n");
                postEvent(MEDIA_PREPARED, 0, 0);
                break;
            case Message::MSG_COMPLETED:
                ALOGD("MSG_COMPLETED:\n");
                postEvent(MEDIA_PLAYBACK_COMPLETE, 0, 0);
                break;
            case Message::MSG_VIDEO_SIZE_CHANGED:
                ALOGD("MSG_VIDEO_SIZE_CHANGED: %d, %d\n", msg.arg1, msg.arg2);
                postEvent(MEDIA_SET_VIDEO_SIZE, msg.arg1, msg.arg2);
                break;
            case Message::MSG_SAR_CHANGED:
                ALOGD("MSG_SAR_CHANGED: %d, %d\n", msg.arg1, msg.arg2);
                postEvent(MEDIA_SET_VIDEO_SAR, msg.arg1, msg.arg2);
                break;
            case Message::MSG_VIDEO_RENDERING_START:
                ALOGD("MSG_VIDEO_RENDERING_START:\n");
                postEvent(MEDIA_INFO, MEDIA_INFO_VIDEO_RENDERING_START, 0);
                break;
            case Message::MSG_AUDIO_RENDERING_START:
                ALOGD("MSG_AUDIO_RENDERING_START:\n");
                postEvent(MEDIA_INFO, MEDIA_INFO_AUDIO_RENDERING_START, 0);
                break;
            case Message::MSG_VIDEO_ROTATION_CHANGED:
                ALOGD("MSG_VIDEO_ROTATION_CHANGED: %d\n", msg.arg1);
                postEvent(MEDIA_INFO, MEDIA_INFO_VIDEO_ROTATION_CHANGED, msg.arg1);
                break;
            case Message::MSG_AUDIO_DECODED_START:
                ALOGD("MSG_AUDIO_DECODED_START:\n");
                postEvent(MEDIA_INFO, MEDIA_INFO_AUDIO_DECODED_START, 0);
                break;
            case Message::MSG_VIDEO_DECODED_START:
                ALOGD("MSG_VIDEO_DECODED_START:\n");
                postEvent(MEDIA_INFO, MEDIA_INFO_VIDEO_DECODED_START, 0);
                break;
            case Message::MSG_OPEN_INPUT:
                ALOGD("MSG_OPEN_INPUT:\n");
                postEvent(MEDIA_INFO, MEDIA_INFO_OPEN_INPUT, 0);
                break;
            case Message::MSG_FIND_STREAM_INFO:
                ALOGD("MSG_FIND_STREAM_INFO:\n");
                postEvent(MEDIA_INFO, MEDIA_INFO_FIND_STREAM_INFO, 0);
                break;
            case Message::MSG_COMPONENT_OPEN:
                ALOGD("MSG_COMPONENT_OPEN:\n");
                postEvent(MEDIA_INFO, MEDIA_INFO_COMPONENT_OPEN, 0);
                break;
            case Message::MSG_BUFFERING_START:
                ALOGD("MSG_BUFFERING_START:\n");
                postEvent(MEDIA_INFO, MEDIA_INFO_BUFFERING_START, msg.arg1);
                break;
            case Message::MSG_BUFFERING_END:
                ALOGD("MSG_BUFFERING_END:\n");
                postEvent(MEDIA_INFO, MEDIA_INFO_BUFFERING_END, msg.arg1);
                break;
            case Message::MSG_BUFFERING_UPDATE:
                ALOGD("MSG_BUFFERING_UPDATE: %d, %d\n", msg.arg1, msg.arg2);
                postEvent(MEDIA_BUFFERING_UPDATE, msg.arg1, msg.arg2);
                break;
            case Message::MSG_BUFFERING_BYTES_UPDATE:
                break;
            case Message::MSG_BUFFERING_TIME_UPDATE:
                break;
            case Message::MSG_SEEK_COMPLETE:
                ALOGD("MSG_SEEK_COMPLETE:\n");
                postEvent(MEDIA_SEEK_COMPLETE, 0, 0);
                break;
            case Message::MSG_ACCURATE_SEEK_COMPLETE:
                ALOGD("MSG_ACCURATE_SEEK_COMPLETE:\n");
                postEvent(MEDIA_INFO, MEDIA_INFO_MEDIA_ACCURATE_SEEK_COMPLETE, msg.arg1);
                break;
            case Message::MSG_PLAYBACK_STATE_CHANGED:
                break;
            case Message::MSG_TIMED_TEXT:
                ALOGD("MSG_TIMED_TEXT:\n");
                if (msg.obj) {
//                    jstring text = (*env)->NewStringUTF(env, (char *)msg.obj);
                    postEvent2(MEDIA_TIMED_TEXT, 0, 0, NULL);
//                    J4A_DeleteLocalRef__p(env, &text);
                } else {
                    postEvent2(MEDIA_TIMED_TEXT, 0, 0, NULL);
                }
                break;
            case Message::MSG_GET_IMG_STATE:
                ALOGD("MSG_GET_IMG_STATE:\n");
                if (msg.obj) {
//                    jstring file_name = (*env)->NewStringUTF(env, (char *)msg.obj);
                    postEvent2(MEDIA_GET_IMG_STATE, msg.arg1, msg.arg2, NULL);
//                    J4A_DeleteLocalRef__p(env, &file_name);
                } else {
                    postEvent2(MEDIA_GET_IMG_STATE, msg.arg1, msg.arg2, NULL);
                }
                break;
            case Message::MSG_VIDEO_SEEK_RENDERING_START:
                ALOGD("MSG_VIDEO_SEEK_RENDERING_START:\n");
                postEvent(MEDIA_INFO, MEDIA_INFO_VIDEO_SEEK_RENDERING_START, msg.arg1);
                break;
            case Message::MSG_AUDIO_SEEK_RENDERING_START:
                ALOGD("MSG_AUDIO_SEEK_RENDERING_START:\n");
                postEvent(MEDIA_INFO, MEDIA_INFO_AUDIO_SEEK_RENDERING_START, msg.arg1);
                break;
            default:
                ALOGE("unknown MSG_xxx(%d)\n", msg.what);
                break;
        }

        msg.free();
    }
    return EXIT_FAILURE;
}

void AndroidMediaPlayer::postEvent(int what, int arg1, int arg2) {
    ALOGD("%s what=%d arg1=%d arg2=%d", __func__, what, arg1, arg2);
}

void AndroidMediaPlayer::postEvent2(int what, int arg1, int arg2, jobject obj) {
    ALOGD("%s what=%d arg1=%d arg2=%d %p", __func__, what, arg1, arg2, &obj);
}

