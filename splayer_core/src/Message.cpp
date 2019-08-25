#include "Message.h"

Message::Message() = default;

Message::~Message() {
    free();
}

const char *Message::getMsgSimpleName(int what) {
    switch (what) {
        case MSG_FLUSH :
            return "MSG_FLUSH";
        case MSG_ERROR :
            return "MSG_ERROR";
        case MSG_PREPARED :
            return "MSG_PREPARED";
        case MSG_COMPLETED :
            return "MSG_COMPLETED";
        case MSG_VIDEO_SIZE_CHANGED :
            return "MSG_VIDEO_SIZE_CHANGED";
        case MSG_SAR_CHANGED :
            return "MSG_SAR_CHANGED";
        case MSG_VIDEO_RENDERING_START :
            return "MSG_VIDEO_RENDERING_START";
        case MSG_AUDIO_RENDERING_START :
            return "MSG_AUDIO_RENDERING_START";
        case MSG_VIDEO_ROTATION_CHANGED :
            return "MSG_VIDEO_ROTATION_CHANGED";
        case MSG_AUDIO_DECODED_START :
            return "MSG_AUDIO_DECODED_START";
        case MSG_VIDEO_DECODED_START :
            return "MSG_VIDEO_DECODED_START";
        case MSG_OPEN_INPUT :
            return "MSG_OPEN_INPUT";
        case MSG_FIND_STREAM_INFO :
            return "MSG_FIND_STREAM_INFO";
        case MSG_COMPONENTS_OPEN :
            return "MSG_COMPONENTS_OPEN";
        case MSG_VIDEO_SEEK_RENDERING_START :
            return "MSG_VIDEO_SEEK_RENDERING_START";
        case MSG_AUDIO_SEEK_RENDERING_START :
            return "MSG_AUDIO_SEEK_RENDERING_START";
        case MSG_BUFFERING_START :
            return "MSG_BUFFERING_START";
        case MSG_BUFFERING_END :
            return "MSG_BUFFERING_END";
        case MSG_BUFFERING_UPDATE :
            return "MSG_BUFFERING_UPDATE";
        case MSG_BUFFERING_BYTES_UPDATE :
            return "MSG_BUFFERING_BYTES_UPDATE";
        case MSG_BUFFERING_TIME_UPDATE :
            return "MSG_BUFFERING_TIME_UPDATE";
        case MSG_SEEK_COMPLETE :
            return "MSG_SEEK_COMPLETE";
        case MSG_STATE_CHANGED :
            return "MSG_STATE_CHANGED";
        case MSG_TIMED_TEXT :
            return "MSG_TIMED_TEXT";
        case MSG_ACCURATE_SEEK_COMPLETE :
            return "MSG_ACCURATE_SEEK_COMPLETE";
        case MSG_GET_IMG_STATE :
            return "MSG_GET_IMG_STATE";
        case REQ_START :
            return "REQ_START";
        case REQ_PAUSE :
            return "REQ_PAUSE";
        case REQ_SEEK :
            return "REQ_SEEK";
        case PROP_FLOAT_VIDEO_DECODE_FRAMES_PER_SECOND :
            return "PROP_FLOAT_VIDEO_DECODE_FRAMES_PER_SECOND";
        case PROP_FLOAT_VIDEO_OUTPUT_FRAMES_PER_SECOND :
            return "PROP_FLOAT_VIDEO_OUTPUT_FRAMES_PER_SECOND";
        case PROP_FLOAT_PLAYBACK_RATE :
            return "PROP_FLOAT_PLAYBACK_RATE";
        case PROP_FLOAT_PLAYBACK_VOLUME :
            return "PROP_FLOAT_PLAYBACK_VOLUME";
        case PROP_FLOAT_AVDELAY :
            return "PROP_FLOAT_AVDELAY";
        case PROP_FLOAT_AVDIFF :
            return "PROP_FLOAT_AVDIFF";
        case PROP_FLOAT_DROP_FRAME_RATE :
            return "PROP_FLOAT_DROP_FRAME_RATE";
        case PROP_INT64_SELECTED_VIDEO_STREAM :
            return "PROP_INT64_SELECTED_VIDEO_STREAM";
        case PROP_INT64_SELECTED_AUDIO_STREAM :
            return "PROP_INT64_SELECTED_AUDIO_STREAM";
        case PROP_INT64_SELECTED_TIMEDTEXT_STREAM :
            return "PROP_INT64_SELECTED_TIMEDTEXT_STREAM";
        case PROP_INT64_VIDEO_DECODER :
            return "PROP_INT64_VIDEO_DECODER";
        case PROP_INT64_AUDIO_DECODER :
            return "PROP_INT64_AUDIO_DECODER";
        case PROP_V_DECODER_UNKNOWN :
            return "PROP_V_DECODER_UNKNOWN";
        case PROP_V_DECODER_AVCODEC :
            return "PROP_V_DECODER_AVCODEC";
        case PROP_V_DECODER_MEDIACODEC :
            return "PROP_V_DECODER_MEDIACODEC";
        case PROP_V_DECODER_VIDEOTOOLBOX :
            return "PROP_V_DECODER_VIDEOTOOLBOX";
        case PROP_INT64_VIDEO_CACHED_DURATION :
            return "PROP_INT64_VIDEO_CACHED_DURATION";
        case PROP_INT64_AUDIO_CACHED_DURATION :
            return "PROP_INT64_AUDIO_CACHED_DURATION";
        case PROP_INT64_VIDEO_CACHED_BYTES :
            return "PROP_INT64_VIDEO_CACHED_BYTES";
        case PROP_INT64_AUDIO_CACHED_BYTES :
            return "PROP_INT64_AUDIO_CACHED_BYTES";
        case PROP_INT64_VIDEO_CACHED_PACKETS :
            return "PROP_INT64_VIDEO_CACHED_PACKETS";
        case PROP_INT64_AUDIO_CACHED_PACKETS :
            return "PROP_INT64_AUDIO_CACHED_PACKETS";
        case PROP_INT64_BIT_RATE :
            return "PROP_INT64_BIT_RATE";
        case PROP_INT64_TCP_SPEED :
            return "PROP_INT64_TCP_SPEED";
        case PROP_INT64_ASYNC_STATISTIC_BUF_BACKWARDS :
            return "PROP_INT64_ASYNC_STATISTIC_BUF_BACKWARDS";
        case PROP_INT64_ASYNC_STATISTIC_BUF_FORWARDS :
            return "PROP_INT64_ASYNC_STATISTIC_BUF_FORWARDS";
        case PROP_INT64_ASYNC_STATISTIC_BUF_CAPACITY :
            return "PROP_INT64_ASYNC_STATISTIC_BUF_CAPACITY";
        case PROP_INT64_TRAFFIC_STATISTIC_BYTE_COUNT :
            return "PROP_INT64_TRAFFIC_STATISTIC_BYTE_COUNT";
        case PROP_INT64_LATEST_SEEK_LOAD_DURATION :
            return "PROP_INT64_LATEST_SEEK_LOAD_DURATION";
        case PROP_INT64_CACHE_STATISTIC_PHYSICAL_POS :
            return "PROP_INT64_CACHE_STATISTIC_PHYSICAL_POS";
        case PROP_INT64_CACHE_STATISTIC_FILE_FORWARDS :
            return "PROP_INT64_CACHE_STATISTIC_FILE_FORWARDS";
        case PROP_INT64_CACHE_STATISTIC_FILE_POS :
            return "PROP_INT64_CACHE_STATISTIC_FILE_POS";
        case PROP_INT64_CACHE_STATISTIC_COUNT_BYTES :
            return "PROP_INT64_CACHE_STATISTIC_COUNT_BYTES";
        case PROP_INT64_LOGICAL_FILE_SIZE :
            return "PROP_INT64_LOGICAL_FILE_SIZE";
        case PROP_INT64_SHARE_CACHE_DATA :
            return "PROP_INT64_SHARE_CACHE_DATA";
        case PROP_INT64_IMMEDIATE_RECONNECT :
            return "PROP_INT64_IMMEDIATE_RECONNECT";
        case MSG_VIDEO_DECODER_OPEN:
            return "MSG_VIDEO_DECODER_OPEN";
        case MSG_AUDIO_COMPONENT_OPEN:
            return "MSG_AUDIO_COMPONENT_OPEN";
        case MSG_VIDEO_COMPONENT_OPEN:
            return "MSG_VIDEO_COMPONENT_OPEN";
        case MSG_SUBTITLE_COMPONENT_OPEN:
            return "MSG_SUBTITLE_COMPONENT_OPEN";
        case MSG_OPTIONS_CREATED:
            return "MSG_OPTIONS_CREATED";
        case MSG_SURFACE_CREATED:
            return "MSG_SURFACE_CREATED";
        case MSG_AUDIO_CREATED:
            return "MSG_AUDIO_CREATED";
        case MSG_STREAM_CREATED:
            return "MSG_STREAM_CREATED";
        case MSG_STREAM_FAILURE:
            return "MSG_STREAM_FAILURE";
        case MSG_EVENT_CREATED:
            return "MSG_EVENT_CREATED";
        case REQ_QUIT:
            return "REQ_QUIT";
        default:
            return "NONE";
    }
}

void Message::free() {
    what = -1;
    arg1 = -1;
    arg2 = -1;
    if (obj) {
        obj->destroy();
        obj = nullptr;
    }
}
