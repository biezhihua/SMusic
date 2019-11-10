import Foundation

enum MsgType: Int {

    // 默认
    case MSG_FLUSH = 1000
    // 出错
    case MSG_ERROR = 1001
    // 改变状态
    case MSG_CHANGE_STATUS = 1002
    // 播放开始
    case MSG_PLAY_STARTED = 1003
    // 播放完成
    case MSG_PLAY_COMPLETED = 1004
    // 打开文件
    case MSG_OPEN_INPUT = 1005
    // 媒体流信息
    case MSG_STREAM_INFO = 1006
    // 已准备解码器
    case MSG_PREPARED_DECODER = 1007
    // 长宽比变化
    case MSG_VIDEO_SIZE_CHANGED = 1008
    // 采样率变化
    case MSG_SAR_CHANGED = 1009
    // 开始音频解码
    case MSG_AUDIO_START = 1010
    // 音频渲染开始=播放开始
    case MSG_AUDIO_RENDERING_START = 1011
    // 视频渲染开始=渲染开始
    case MSG_VIDEO_START = 1012
    // 旋转角度变化
    case MSG_VIDEO_ROTATION_CHANGED = 1013
    // 缓冲开始
    case MSG_BUFFERING_START = 1014
    // 缓冲更新
    case MSG_BUFFERING_UPDATE = 1015
    // 缓冲时间更新
    case MSG_BUFFERING_TIME_UPDATE = 1016
    // 缓冲完成
    case MSG_BUFFERING_END = 1017
    // 定位完成
    case MSG_SEEK_START = 1018
    // 定位开始
    case MSG_SEEK_COMPLETE = 1019
    // 字幕
    case MSG_TIMED_TEXT = 1020
    // 当前时钟
    case MSG_CURRENT_POSITION = 1021
}
