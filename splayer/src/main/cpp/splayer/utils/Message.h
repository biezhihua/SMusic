//
// Created by biezhihua on 2019-06-13.
//

#ifndef SPLAYER_MESSAGE_H
#define SPLAYER_MESSAGE_H


class Message {
public:
    Message();

    ~Message();

    int what;
    int arg1;
    int arg2;
    void *obj;

    static const int FFP_MSG_FLUSH = 0;
    static const int FFP_MSG_ERROR = 100;    /* arg1 = error */
    static const int FFP_MSG_PREPARED = 200;
    static const int FFP_MSG_COMPLETED = 300;
    static const int FFP_MSG_VIDEO_SIZE_CHANGED = 400;     /* arg1 = width, arg2 = height */
    static const int FFP_MSG_SAR_CHANGED = 401;    /* arg1 = sar.num, arg2 = sar.den */
    static const int FFP_MSG_VIDEO_RENDERING_START = 402;
    static const int FFP_MSG_AUDIO_RENDERING_START = 403;
    static const int FFP_MSG_VIDEO_ROTATION_CHANGED = 404;    /* arg1 = degree */
    static const int FFP_MSG_AUDIO_DECODED_START = 405;
    static const int FFP_MSG_VIDEO_DECODED_START = 406;
    static const int FFP_MSG_OPEN_INPUT = 407;
    static const int FFP_MSG_FIND_STREAM_INFO = 408;
    static const int FFP_MSG_COMPONENT_OPEN = 409;
    static const int FFP_MSG_VIDEO_SEEK_RENDERING_START = 410;
    static const int FFP_MSG_AUDIO_SEEK_RENDERING_START = 411;

    static const int FFP_MSG_BUFFERING_START = 500;
    static const int FFP_MSG_BUFFERING_END = 501;
    static const int FFP_MSG_BUFFERING_UPDATE = 502;    /* arg1 = buffering head position in time, arg2 = minimum percent in time or bytes */
    static const int FFP_MSG_BUFFERING_BYTES_UPDATE = 503;   /* arg1 = cached data in bytes,            arg2 = high water mark */
    static const int FFP_MSG_BUFFERING_TIME_UPDATE = 504;   /* arg1 = cached duration in milliseconds, arg2 = high water mark */
    static const int FFP_MSG_SEEK_COMPLETE = 600;   /* arg1 = seek position,                   arg2 = error */
    static const int FFP_MSG_PLAYBACK_STATE_CHANGED = 700;
    static const int FFP_MSG_TIMED_TEXT = 800;
    static const int FFP_MSG_ACCURATE_SEEK_COMPLETE = 900;    /* arg1 = current position*/
    static const int FFP_MSG_GET_IMG_STATE = 1000;   /* arg1 = timestamp, arg2 = result code, obj = file name*/

    static const int FFP_MSG_VIDEO_DECODER_OPEN = 10001;

    static const int FFP_REQ_START = 20001;
    static const int FFP_REQ_PAUSE = 20002;
    static const int FFP_REQ_SEEK = 20003;

    static const int FFP_PROP_FLOAT_VIDEO_DECODE_FRAMES_PER_SECOND = 10001;
    static const int FFP_PROP_FLOAT_VIDEO_OUTPUT_FRAMES_PER_SECOND = 10002;
    static const int FFP_PROP_FLOAT_PLAYBACK_RATE = 10003;
    static const int FFP_PROP_FLOAT_PLAYBACK_VOLUME = 10006;
    static const int FFP_PROP_FLOAT_AVDELAY = 10004;
    static const int FFP_PROP_FLOAT_AVDIFF = 10005;
    static const int FFP_PROP_FLOAT_DROP_FRAME_RATE = 10007;

    static const int FFP_PROP_INT64_SELECTED_VIDEO_STREAM = 20001;
    static const int FFP_PROP_INT64_SELECTED_AUDIO_STREAM = 20002;
    static const int FFP_PROP_INT64_SELECTED_TIMEDTEXT_STREAM = 20011;
    static const int FFP_PROP_INT64_VIDEO_DECODER = 20003;
    static const int FFP_PROP_INT64_AUDIO_DECODER = 20004;
    static const int FFP_PROPV_DECODER_UNKNOWN = 0;
    static const int FFP_PROPV_DECODER_AVCODEC = 1;
    static const int FFP_PROPV_DECODER_MEDIACODEC = 2;
    static const int FFP_PROPV_DECODER_VIDEOTOOLBOX = 3;
    static const int FFP_PROP_INT64_VIDEO_CACHED_DURATION = 20005;
    static const int FFP_PROP_INT64_AUDIO_CACHED_DURATION = 20006;
    static const int FFP_PROP_INT64_VIDEO_CACHED_BYTES = 20007;
    static const int FFP_PROP_INT64_AUDIO_CACHED_BYTES = 20008;
    static const int FFP_PROP_INT64_VIDEO_CACHED_PACKETS = 20009;
    static const int FFP_PROP_INT64_AUDIO_CACHED_PACKETS = 20010;

    static const int FFP_PROP_INT64_BIT_RATE = 20100;

    static const int FFP_PROP_INT64_TCP_SPEED = 20200;

    static const int FFP_PROP_INT64_ASYNC_STATISTIC_BUF_BACKWARDS = 20201;
    static const int FFP_PROP_INT64_ASYNC_STATISTIC_BUF_FORWARDS = 20202;
    static const int FFP_PROP_INT64_ASYNC_STATISTIC_BUF_CAPACITY = 20203;
    static const int FFP_PROP_INT64_TRAFFIC_STATISTIC_BYTE_COUNT = 20204;

    static const int FFP_PROP_INT64_LATEST_SEEK_LOAD_DURATION = 20300;

    static const int FFP_PROP_INT64_CACHE_STATISTIC_PHYSICAL_POS = 20205;

    static const int FFP_PROP_INT64_CACHE_STATISTIC_FILE_FORWARDS = 20206;

    static const int FFP_PROP_INT64_CACHE_STATISTIC_FILE_POS = 20207;

    static const int FFP_PROP_INT64_CACHE_STATISTIC_COUNT_BYTES = 20208;

    static const int FFP_PROP_INT64_LOGICAL_FILE_SIZE = 20209;
    static const int FFP_PROP_INT64_SHARE_CACHE_DATA = 20210;
    static const int FFP_PROP_INT64_IMMEDIATE_RECONNECT = 20211;

};

#endif //SPLAYER_MESSAGE_H
