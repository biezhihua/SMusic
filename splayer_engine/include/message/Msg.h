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
    static const int MSG_STATUS_STOPED = 6;

    /// 已销毁态
    static const int MSG_STATUS_DESTROYED = 7;

    /// 已错误态
    static const int MSG_STATUS_ERRORED = 8;


    /////////////////////////////////////////////
    /////////////////////////////////////////////
    ///  消息类型范围 1000~9999

    /// 默认
    static const int MSG_FLUSH = 1000;

    /// 出错
    static const int MSG_ERROR = 1001;

    /// 创建
    static const int MSG_CREATE = 1002;

    /// 启动
    static const int MSG_START = 1003;

    /// 已经开始
    static const int MSG_STARTED = 1004;

    /// 暂停
    static const int MSG_PAUSE = 1005;

    /// 播放
    static const int MSG_PLAY = 1006;

    /// 停止
    static const int MSG_STOP = 1007;

    /// 已停止
    static const int MSG_STOPED = 1008;

    /// 销毁
    static const int MSG_DESTROY = 1009;

    /// 开始音视频同步
    static const int MSG_MEDIA_SYNC_START = 10010;

    /// 停止音视频同步
    static const int MSG_MEDIA_SYNC_STOP = 10011;

    /// 音频设备启动
    static const int MSG_AUDIO_DEVICE_START = 10012;

    /// 音频设备停止
    static const int MSG_AUDIO_DEVICE_STOP = 10013;

    /// 视频设备启动
    static const int MSG_VIDEO_DEVICE_START = 10014;

    /// 视频设备停止
    static const int MSG_VIDEO_DEVICE_STOP = 10015;

    /// 视频解码线程启动
    static const int MSG_VIDEO_DECODER_START = 10016;

    /// 视频解码线程停止
    static const int MSG_VIDEO_DECODER_STOP = 10018;

    /// 音频解码线程启动
    static const int MSG_AUDIO_DECODER_START = 10019;

    /// 音频解码线程停止
    static const int MSG_AUDIO_DECODER_STOP = 10020;

    /// 开始读取包
    static const int MSG_MEDIA_STREAM_START = 10021;

    /// 停止读取包
    static const int MSG_MEDIA_STREAM_STOP = 10022;

    /////////////////////////////////////////////
    /////////////////////////////////////////////
    ///  请求消息范围 20000 ~ 29999

    /// 请求开始
    static const int MSG_REQUEST_START = 20001;

    /// 请求停止
    static const int MSG_REQUEST_STOP = 20002;

    /// 请求播放或者暂停
    static const int MSG_REQUEST_PLAY_OR_PAUSE = 20003;

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

    /// 当前时钟
    static const int MSG_CURRENT_POSITON = 30000;

    /// 打开音视频设备失败
    static const int MSG_NOT_OPEN_AUDIO_DEVICE = 30003;

    /////////////////////////////////////////////
    /////////////////////////////////////////////
    ///  错误消息范围 40000 ~ 49999

    static const char *getMsgSimpleName(int what);

};

#endif
