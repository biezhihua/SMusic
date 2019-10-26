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
    ///  消息类型范围 0~9999

    /// 默认
    static const int MSG_FLUSH = 0;

    /// 出错
    static const int MSG_ERROR = 1;

    /// 创建
    static const int MSG_CREATE = 2;

    /// 启动
    static const int MSG_START = 3;

    /// 已经开始
    static const int MSG_STARTED = 4;

    /// 暂停
    static const int MSG_PAUSE = 5;

    /// 播放
    static const int MSG_PLAY = 6;

    /// 停止
    static const int MSG_STOP = 7;

    /// 已停止
    static const int MSG_STOPED = 8;

    /// 销毁
    static const int MSG_DESTROY = 9;

    /// 开始音视频同步
    static const int MSG_MEDIA_SYNC_START = 10;

    /// 停止音视频同步
    static const int MSG_MEDIA_SYNC_STOP = 11;

    /// 音频设备启动
    static const int MSG_AUDIO_DEVICE_START = 12;

    /// 音频设备停止
    static const int MSG_AUDIO_DEVICE_STOP = 13;

    /// 视频设备启动
    static const int MSG_VIDEO_DEVICE_START = 14;

    /// 视频设备停止
    static const int MSG_VIDEO_DEVICE_STOP = 15;

    /// 视频解码线程启动
    static const int MSG_VIDEO_DECODER_START = 16;

    /// 视频解码线程停止
    static const int MSG_VIDEO_DECODER_STOP = 18;

    /// 音频解码线程启动
    static const int MSG_AUDIO_DECODER_START = 19;

    /// 音频解码线程停止
    static const int MSG_AUDIO_DECODER_STOP = 20;

    /// 开始读取包
    static const int MSG_MEDIA_STREAM_START = 21;

    /// 停止读取包
    static const int MSG_MEDIA_STREAM_STOP = 22;

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
