#ifndef ENGINE_MSG_H
#define ENGINE_MSG_H

class Msg {

public:

    Msg();

    ~Msg();

    int what = -1;
    int arg1I = -1;
    int arg2I = -1;
    float arg1F = -1.F;
    float arg2F = -1.F;

    void free();

    /////////////////////////////////////////////
    /////////////////////////////////////////////
    ///  消息类型范围 0~999

    /// 已闲置态
    static const int MSG_STATUS_IDLED = 1;

    /// 已创建态
    static const int MSG_STATUS_CREATED = 2;

    /// 已启动态
    static const int MSG_STATUS_STARTED = 3;

    /// 已播放态
    static const int MSG_STATUS_PLAYING = 4;

    /// 已暂停态
    static const int MSG_STATUS_PAUSED = 5;

    /// 已停止态
    static const int MSG_STATUS_STOPPED = 6;

    /// 已销毁态
    static const int MSG_STATUS_DESTROYED = 7;

    /// 已错误态
    static const int MSG_STATUS_ERRORED = 8;

    /// 准备启动
    static const int MSG_STATUS_PREPARE_START = 10;

    /// 准备停止
    static const int MSG_STATUS_PREPARE_STOP = 11;

    /// 准备销毁
    static const int MSG_STATUS_PREPARE_DESTROY = 12;


    /////////////////////////////////////////////
    /////////////////////////////////////////////
    ///  消息类型范围 1000~9999

    /// 默认
    static const int MSG_FLUSH = 1000;

    /// 出错
    static const int MSG_ERROR = 1001;

    /// 改变消息状态
    static const int MSG_CHANGE_STATUS = 1002;

    /// 开始音视频同步
    static const int MSG_MEDIA_SYNC_START = 1010;

    /// 停止音视频同步
    static const int MSG_MEDIA_SYNC_STOP = 1011;

    /// 音频设备启动
    static const int MSG_AUDIO_DEVICE_START = 1012;

    /// 音频设备停止
    static const int MSG_AUDIO_DEVICE_STOP = 1013;

    /// 视频设备启动
    static const int MSG_VIDEO_DEVICE_START = 1014;

    /// 视频设备停止
    static const int MSG_VIDEO_DEVICE_STOP = 1015;

    /// 视频解码线程启动
    static const int MSG_VIDEO_DECODER_START = 1016;

    /// 视频解码线程停止
    static const int MSG_VIDEO_DECODER_STOP = 1018;

    /// 音频解码线程启动
    static const int MSG_AUDIO_DECODER_START = 1019;

    /// 音频解码线程停止
    static const int MSG_AUDIO_DECODER_STOP = 1020;

    /// 开始读取包
    static const int MSG_MEDIA_STREAM_START = 1021;

    /// 停止读取包
    static const int MSG_MEDIA_STREAM_STOP = 1022;

    /////////////////////////////////////////////
    /////////////////////////////////////////////
    ///  请求消息范围 20000 ~ 29999

    /// 请求开始
    static const int MSG_REQUEST_START = 20001;

    /// 请求停止
    static const int MSG_REQUEST_STOP = 20002;

    /// 请求创建
    static const int MSG_REQUEST_CREATE = 20004;

    /// 请求销毁
    static const int MSG_REQUEST_DESTROY = 20005;

    /// 请求定位
    static const int MSG_REQUEST_SEEK = 20006;

    /// 请求退出
    static const int MSG_REQUEST_QUIT = 20007;

    /// 请求错误
    static const int MSG_REQUEST_ERROR = 20008;

    /// 请求暂停
    static const int MSG_REQUEST_PAUSE = 20009;

    /// 请求播放
    static const int MSG_REQUEST_PLAY = 20010;

    /////////////////////////////////////////////
    /////////////////////////////////////////////
    ///  扩展消息范围 30000 ~ 39999

    /////////////////////////////////////////////
    /////////////////////////////////////////////
    ///  错误消息范围 40000 ~ 49999

    static const char *getMsgSimpleName(int what);

};

#endif
