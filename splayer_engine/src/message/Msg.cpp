#include <message/Msg.h>

Msg::Msg() = default;

Msg::~Msg() {
    free();
}

void Msg::free() {
    what = -1;
    arg1I = -1;
    arg2I = -1;
    arg1F = -1.F;
    arg2F = -1.F;
}

const char *Msg::getMsgSimpleName(int what) {
    switch (what) {
        case MSG_FLUSH:
            return "MSG_FLUSH";
        case MSG_ERROR:
            return "MSG_ERROR";
        case MSG_STATUS_IDLED:
            return "MSG_STATUS_IDLED";
        case MSG_STATUS_CREATED:
            return "MSG_STATUS_CREATED";
        case MSG_STATUS_STARTED:
            return "MSG_STATUS_STARTED";
        case MSG_STATUS_PLAYING:
            return "MSG_STATUS_PLAYING";
        case MSG_STATUS_PAUSED:
            return "MSG_STATUS_PAUSED";
        case MSG_STATUS_STOPPED:
            return "MSG_STATUS_STOPPED";
        case MSG_STATUS_DESTROYED:
            return "MSG_STATUS_DESTROYED";
        case MSG_STATUS_ERRORED:
            return "MSG_STATUS_ERRORED";
        case MSG_STATUS_PREPARE_START:
            return "MSG_STATUS_PREPARE_START";
        case MSG_STATUS_PREPARE_STOP:
            return "MSG_STATUS_PREPARE_STOP";
        case MSG_STATUS_PREPARE_DESTROY:
            return "MSG_STATUS_PREPARE_DESTROY";

            ///

        case MSG_PLAY_STARTED:
            return "MSG_PLAY_STARTED";
        case MSG_PLAY_COMPLETED:
            return "MSG_PLAY_COMPLETED";
        case MSG_OPEN_INPUT:
            return "MSG_OPEN_INPUT";
        case MSG_STREAM_INFO:
            return "MSG_STREAM_INFO";
        case MSG_PREPARED_DECODER:
            return "MSG_PREPARED_DECODER";
        case MSG_VIDEO_SIZE_CHANGED:
            return "MSG_VIDEO_SIZE_CHANGED";
        case MSG_SAR_CHANGED:
            return "MSG_SAR_CHANGED";
        case MSG_AUDIO_START:
            return "MSG_AUDIO_START";
        case MSG_AUDIO_RENDERING_START:
            return "MSG_AUDIO_RENDERING_START";
        case MSG_VIDEO_START:
            return "MSG_VIDEO_START";
        case MSG_VIDEO_ROTATION_CHANGED:
            return "MSG_VIDEO_ROTATION_CHANGED";
        case MSG_BUFFERING_START:
            return "MSG_BUFFERING_START";
        case MSG_BUFFERING_UPDATE:
            return "MSG_BUFFERING_UPDATE";
        case MSG_BUFFERING_TIME_UPDATE:
            return "MSG_BUFFERING_TIME_UPDATE";
        case MSG_BUFFERING_END:
            return "MSG_BUFFERING_END";
        case MSG_SEEK_COMPLETE:
            return "MSG_SEEK_COMPLETE";
        case MSG_TIMED_TEXT:
            return "MSG_TIMED_TEXT";
        case MSG_CURRENT_POSITON:
            return "MSG_CURRENT_POSITON";

            ///

        case MSG_REQUEST_START:
            return "MSG_REQUEST_START";
        case MSG_REQUEST_STOP:
            return "MSG_REQUEST_STOP";
        case MSG_REQUEST_CREATE:
            return "MSG_REQUEST_CREATE";
        case MSG_REQUEST_DESTROY:
            return "MSG_REQUEST_DESTROY";
        case MSG_REQUEST_SEEK:
            return "MSG_REQUEST_SEEK";
        case MSG_REQUEST_QUIT:
            return "MSG_REQUEST_QUIT";
        case MSG_REQUEST_PAUSE:
            return "MSG_REQUEST_PAUSE";
        case MSG_REQUEST_PLAY:
            return "MSG_REQUEST_PLAY";

            ////

        case MSG_CHANGE_STATUS:
            return "MSG_CHANGE_STATUS";

        default:
            return "NONE";
    }
}
