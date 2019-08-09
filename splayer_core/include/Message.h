#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#ifndef SPLAYER_MESSAGE_H
#define SPLAYER_MESSAGE_H

class MessageObject {
public:
    virtual int destroy() = 0;
};

class Message {
public:
    Message();

    ~Message();

    int what = -1;
    int arg1 = -1;
    int arg2 = -1;
    MessageObject *obj = nullptr;

    void free();

    /* 0~9999 */

    static const int MSG_FLUSH = 0;
    static const int MSG_ERROR = 1; /* arg1 = error */
    static const int MSG_PREPARED = 2;
    static const int MSG_COMPLETED = 3;
    static const int MSG_VIDEO_SIZE_CHANGED = 4; /* arg1 = width, arg2 = height */
    static const int MSG_SAR_CHANGED = 5;        /* arg1 = sampleAspectRatio.num, arg2 = sampleAspectRatio.den */
    static const int MSG_VIDEO_RENDERING_START = 6;
    static const int MSG_AUDIO_RENDERING_START = 7;
    static const int MSG_VIDEO_ROTATION_CHANGED = 8; /* arg1 = degree */
    static const int MSG_AUDIO_DECODED_START = 9;
    static const int MSG_VIDEO_DECODED_START = 10;
    static const int MSG_OPEN_INPUT = 11;
    static const int MSG_FIND_STREAM_INFO = 12;
    static const int MSG_COMPONENT_OPEN = 13;
    static const int MSG_VIDEO_SEEK_RENDERING_START = 14;
    static const int MSG_AUDIO_SEEK_RENDERING_START = 15;
    static const int MSG_BUFFERING_START = 16;
    static const int MSG_BUFFERING_END = 17;
    static const int MSG_BUFFERING_UPDATE = 18;       /* arg1 = buffering head position in time, arg2 = minimum percent in time or bytes */
    static const int MSG_BUFFERING_BYTES_UPDATE = 19; /* arg1 = cached data in bytes,            arg2 = high water mark */
    static const int MSG_BUFFERING_TIME_UPDATE = 20;  /* arg1 = cached duration in milliseconds, arg2 = high water mark */
    static const int MSG_SEEK_COMPLETE = 21;          /* arg1 = seek position,                   arg2 = error */
    static const int MSG_PLAYBACK_STATE_CHANGED = 22;
    static const int MSG_TIMED_TEXT = 23;
    static const int MSG_ACCURATE_SEEK_COMPLETE = 24; /* arg1 = current position*/
    static const int MSG_GET_IMG_STATE = 25;         /* arg1 = timestamp, arg2 = result code, obj = file name*/
    static const int MSG_VIDEO_DECODER_OPEN = 26;

    /* REQ 20000 ~ 29999 */

    static const int REQ_START = 20001;
    static const int REQ_PAUSE = 20002;
    static const int REQ_SEEK = 20003;

    /* PROP 30000~39999 */

    static const int PROP_FLOAT_VIDEO_DECODE_FRAMES_PER_SECOND = 30001;
    static const int PROP_FLOAT_VIDEO_OUTPUT_FRAMES_PER_SECOND = 30002;
    static const int PROP_FLOAT_PLAYBACK_RATE = 30003;
    static const int PROP_FLOAT_AVDELAY = 30004;
    static const int PROP_FLOAT_AVDIFF = 30005;
    static const int PROP_FLOAT_PLAYBACK_VOLUME = 30006;
    static const int PROP_FLOAT_DROP_FRAME_RATE = 30007;
    static const int PROP_INT64_SELECTED_VIDEO_STREAM = 30008;
    static const int PROP_INT64_SELECTED_AUDIO_STREAM = 30009;
    static const int PROP_INT64_SELECTED_TIMEDTEXT_STREAM = 3010;
    static const int PROP_INT64_VIDEO_DECODER = 30011;
    static const int PROP_INT64_AUDIO_DECODER = 30012;
    static const int PROP_V_DECODER_UNKNOWN = 30013;
    static const int PROP_V_DECODER_AVCODEC = 30014;
    static const int PROP_V_DECODER_MEDIACODEC = 30015;
    static const int PROP_V_DECODER_VIDEOTOOLBOX = 30016;
    static const int PROP_INT64_VIDEO_CACHED_DURATION = 30017;
    static const int PROP_INT64_AUDIO_CACHED_DURATION = 30018;
    static const int PROP_INT64_VIDEO_CACHED_BYTES = 30019;
    static const int PROP_INT64_AUDIO_CACHED_BYTES = 30020;
    static const int PROP_INT64_VIDEO_CACHED_PACKETS = 30021;
    static const int PROP_INT64_AUDIO_CACHED_PACKETS = 30022;
    static const int PROP_INT64_ASYNC_STATISTIC_BUF_BACKWARDS = 30023;
    static const int PROP_INT64_ASYNC_STATISTIC_BUF_FORWARDS = 30024;
    static const int PROP_INT64_ASYNC_STATISTIC_BUF_CAPACITY = 30025;
    static const int PROP_INT64_TRAFFIC_STATISTIC_BYTE_COUNT = 30026;
    static const int PROP_INT64_LATEST_SEEK_LOAD_DURATION = 30027;
    static const int PROP_INT64_CACHE_STATISTIC_PHYSICAL_POS = 30028;
    static const int PROP_INT64_CACHE_STATISTIC_FILE_FORWARDS = 30029;
    static const int PROP_INT64_CACHE_STATISTIC_FILE_POS = 30030;
    static const int PROP_INT64_CACHE_STATISTIC_COUNT_BYTES = 30031;
    static const int PROP_INT64_LOGICAL_FILE_SIZE = 30032;
    static const int PROP_INT64_SHARE_CACHE_DATA = 30033;
    static const int PROP_INT64_IMMEDIATE_RECONNECT = 30034;
    static const int PROP_INT64_BIT_RATE = 30035;
    static const int PROP_INT64_TCP_SPEED = 30036;

    static const char *getMsgSimpleName(int what);
};

#endif //SPLAYER_MESSAGE_H

#pragma clang diagnostic pop