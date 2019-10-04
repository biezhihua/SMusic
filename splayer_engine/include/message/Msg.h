#ifndef ENGINE_MSG_H
#define ENGINE_MSG_H

class Msg {

public:

    Msg();

    ~Msg();

    int what = -1;
    int arg1 = -1;
    int arg2 = -1;

    void free();

    /////////////////////////////////////////////
    /////////////////////////////////////////////
    ///  消息类型范围 0~9999

    /// 默认
    static const int MSG_FLUSH = 0;

    /// 出错
    static const int MSG_ERROR = 1;

    /// 创建
    static const int MSG_CREATE = 22;

    /// 启动
    static const int MSG_START = 24;

    /// 已经开始
    static const int MSG_STARTED = 3;

    /// 暂停
    static const int MSG_PAUSE = 40;

    /// 播放
    static const int MSG_PLAY = 41;

    /// 停止
    static const int MSG_STOP = 25;

    /// 销毁
    static const int MSG_DESTROY = 23;

    /// 开始音视频同步
    static const int MSG_MEDIA_SYNC_START = 27;

    /// 停止音视频同步
    static const int MSG_MEDIA_SYNC_STOP = 28;

    /// 音频设备启动
    static const int MSG_AUDIO_DEVICE_START = 29;

    /// 音频设备停止
    static const int MSG_AUDIO_DEVICE_STOP = 30;

    /// 视频设备启动
    static const int MSG_VIDEO_DEVICE_START = 31;

    /// 视频设备停止
    static const int MSG_VIDEO_DEVICE_STOP = 32;

    /// 视频解码线程启动
    static const int MSG_VIDEO_DECODER_START = 33;

    /// 视频解码线程停止
    static const int MSG_VIDEO_DECODER_STOP = 36;

    /// 音频解码线程启动
    static const int MSG_AUDIO_DECODER_START = 34;

    /// 音频解码线程停止
    static const int MSG_AUDIO_DECODER_STOP = 35;

    /// 开始读取包
    static const int MSG_MEDIA_STREAM_START = 37;

    /// 停止读取包
    static const int MSG_MEDIA_STREAM_STOP = 38;

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

#endif //ENGINE_MSG_H
