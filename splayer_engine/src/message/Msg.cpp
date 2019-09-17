#include <message/Msg.h>

Msg::Msg() = default;

Msg::~Msg() {
    free();
}

const char *Msg::getMsgSimpleName(int what) {
    switch (what) {
        case MSG_FLUSH:
            return "MSG_FLUSH";
        case MSG_ERROR:
            return "MSG_ERROR";
        case MSG_CREATE:
            return "MSG_CREATE";
        case MSG_START:
            return "MSG_START";
        case MSG_STARTED:
            return "MSG_STARTED";
        case MSG_PLAY:
            return "MSG_PLAY";
        case MSG_PAUSE:
            return "MSG_PAUSE";
        case MSG_STOP:
            return "MSG_STOP";
        case MSG_DESTROY:
            return "MSG_DESTROY";
        case MSG_MEDIA_SYNC_START:
            return "MSG_MEDIA_SYNC_START";
        case MSG_MEDIA_SYNC_STOP:
            return "MSG_MEDIA_SYNC_STOP";
        case MSG_AUDIO_DEVICE_START:
            return "MSG_AUDIO_DEVICE_START";
        case MSG_AUDIO_DEVICE_STOP:
            return "MSG_AUDIO_DEVICE_STOP";
        case MSG_VIDEO_DEVICE_START:
            return "MSG_VIDEO_DEVICE_START";
        case MSG_VIDEO_DEVICE_STOP:
            return "MSG_VIDEO_DEVICE_STOP";
        case MSG_VIDEO_DECODER_START:
            return "MSG_VIDEO_DECODER_START";
        case MSG_VIDEO_DECODER_STOP:
            return "MSG_VIDEO_DECODER_STOP";
        case MSG_AUDIO_DECODER_START:
            return "MSG_AUDIO_DECODER_START";
        case MSG_AUDIO_DECODER_STOP:
            return "MSG_AUDIO_DECODER_STOP";
        case MSG_MEDIA_STREAM_START:
            return "MSG_MEDIA_STREAM_START";
        case MSG_MEDIA_STREAM_STOP:
            return "MSG_MEDIA_STREAM_STOP";

            ///

        case MSG_REQUEST_START:
            return "MSG_REQUEST_START";
        case MSG_REQUEST_STOP:
            return "MSG_REQUEST_STOP";
        case MSG_REQUEST_PLAY_OR_PAUSE:
            return "MSG_REQUEST_PLAY_OR_PAUSE";
        case MSG_REQUEST_CREATE:
            return "MSG_REQUEST_CREATE";
        case MSG_REQUEST_DESTROY:
            return "MSG_REQUEST_DESTROY";
        case MSG_REQUEST_SEEK:
            return "MSG_REQUEST_SEEK";
        case MSG_REQUEST_QUIT:
            return "MSG_REQUEST_QUIT";

            ///

        case MSG_NOT_OPEN_AUDIO_DEVICE:
            return "MSG_NOT_OPEN_AUDIO_DEVICE";

        default:
            return "NONE";
    }
}

void Msg::free() {
    what = -1;
    arg1 = -1;
    arg2 = -1;
}
