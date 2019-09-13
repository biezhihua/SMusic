#include <message/Msg.h>

Msg::Msg() = default;

Msg::~Msg() {
    free();
}

const char *Msg::getMsgSimpleName(int what) {
    switch (what) {
        case MSG_FLUSH:
            return "MSG_FLUSH";
        case MSG_CREATE:
            return "MSG_CREATE";
        case MSG_ERROR:
            return "MSG_ERROR";
        case MSG_PREPARED:
            return "MSG_PREPARED";
        case MSG_VIDEO_DECODER_THREAD_STARTED:
            return "MSG_VIDEO_DECODER_THREAD_STARTED";
        case MSG_AUDIO_DECODER_THREAD_STARTED:
            return "MSG_AUDIO_DECODER_THREAD_STARTED";
        case MSG_NOT_OPEN_AUDIO_DEVICE:
            return "MSG_NOT_OPEN_AUDIO_DEVICE";
        case MSG_AUDIO_DEVICE_STARTED:
            return "MSG_AUDIO_DEVICE_STARTED";
        case MSG_VIDEO_DEVICE_STARTED:
            return "MSG_VIDEO_DEVICE_STARTED";
        default:
            return "NONE";
    }
}

void Msg::free() {
    what = -1;
    arg1 = -1;
    arg2 = -1;
}
