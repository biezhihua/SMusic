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
    /// 状态 消息类型范围 0~999

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
    /// 普通 消息类型范围 1000~9999

    /// 默认
    static const int MSG_FLUSH = 1000;

    /// 出错
    static const int MSG_ERROR = 1001;

    /// 改变状态
    static const int MSG_CHANGE_STATUS = 1002;

    /// 播放开始
    static const int MSG_PLAY_STARTED = 1003;

    /// 播放完成
    static const int MSG_PLAY_COMPLETED = 1004;

    /// 打开文件
    static const int MSG_OPEN_INPUT = 1005;

    /// 媒体流信息
    static const int MSG_STREAM_INFO = 1006;

    /// 已准备解码器
    static const int MSG_PREPARED_DECODER = 1007;

    /// 长宽比变化
    static const int MSG_VIDEO_SIZE_CHANGED = 1008;

    /// 采样率变化
    static const int MSG_SAR_CHANGED = 1009;

    /// 开始音频解码
    static const int MSG_AUDIO_START = 1010;

    /// 音频渲染开始(播放开始)
    static const int MSG_AUDIO_RENDERING_START = 1011;

    /// 视频渲染开始(渲染开始)
    static const int MSG_VIDEO_START = 1012;

    /// 旋转角度变化
    static const int MSG_VIDEO_ROTATION_CHANGED = 1013;

    /// 缓冲开始
    static const int MSG_BUFFERING_START = 1014;

    /// 缓冲更新
    static const int MSG_BUFFERING_UPDATE = 1015;

    /// 缓冲时间更新
    static const int MSG_BUFFERING_TIME_UPDATE = 1016;

    /// 缓冲完成
    static const int MSG_BUFFERING_END = 1017;

    /// 定位完成
    static const int MSG_SEEK_START = 1018;

    /// 定位开始
    static const int MSG_SEEK_COMPLETE = 1019;

    /// 字幕
    static const int MSG_TIMED_TEXT = 1020;

    /// 当前时钟
    static const int MSG_CURRENT_POSITON = 1021;

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
