#ifndef SPLAYER_CORE_VIDEOSTATE_H
#define SPLAYER_CORE_VIDEOSTATE_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
};

#include "FrameQueue.h"
#include "Decoder.h"
#include "PacketQueue.h"
#include "Clock.h"
#include "Thread.h"
#include "AudioParams.h"

class PlayerState {

public:

    ////////////////////////////////////////
    ////////////////////////////////////////
    ////////////////////////////////////////
    /// Common

    /// 输入格式
    AVInputFormat *inputFormat;

    /// 解码格式上下文
    AVFormatContext *formatContext;

    /// 读取帧线程
    Thread *readThread;

    /// 用于控制读取线程
    Mutex *continueReadThread;

    /// 外部时钟
    Clock externalClock;

    /// 显示模式
    int showMode = SHOW_MODE_VIDEO;

    /// 用户退出请求标志
    int abortRequest;

    /// 是否强制刷新
    int forceRefresh;

    /// 音视频同步类型
    /// 1. 按音频为主进行同步
    /// 2. 按视频为主进行同步
    /// set audio-video sync. type (type=audio/video/ext)
    int syncType;

    /// 是否逐帧播放
    int stepFrame;

    SwsContext *imgConvertCtx;

    SwsContext *subConvertCtx;

    /// 帧最大的持续时间
    double maxFrameDuration;

    /// 是否到文件结尾
    int eof;

    /// 队列附件请求
    int queueAttachmentsReq;

    /// 是否实时码流
    int realTime;

    /// 文件名
    char *fileName;

    /// 是否暂停
    int paused;

    /// 上一次暂停状态
    int lastPaused;

    /// 暂停基于网络的流状态
    int readPauseReturn;

    // 宽高与XY轴偏移量
    int width;
    int height;
    int yTop;
    int xLeft;

    /// Seek请求
    int seekReq;

    /// Seek标志
    int seekFlags;

    /// Seek位置
    int64_t seekPos;

    ///
    int64_t seekRel;

    RDFTContext *rdft;
    int rdft_bits;
    FFTSample *rdft_data;

    double lastVisTime;
    double frameTimer;

    double frameSinkFilterStartTime; // 调用av_buffersink_get_frame_flags的开始时间
    double frameSinkFilterConsumeTime; // 调用av_buffersink_get_frame_flags方法的耗时

    int frameDropsEarly; // 早期丢帧数
    int frameDropsLate; // 晚期丢帧数

    ////////////////////////////////////////
    ////////////////////////////////////////
    ////////////////////////////////////////
    /// Subtitle

    FrameQueue subtitleFrameQueue;
    PacketQueue subtitlePacketQueue;
    AVStream *subtitleStream;
    Decoder subtitleDecoder;
    int subtitleLastStreamIndex;
    int subtitleStreamIndex;

    ////////////////////////////////////////
    ////////////////////////////////////////
    ////////////////////////////////////////
    /// Video

    AVStream *videoStream;

    /// 视频队列
    FrameQueue videoFrameQueue;

    /// 视频解码器
    Decoder videoDecoder;

    /// 视频宝队列
    PacketQueue videoPacketQueue;

    /// 视频时钟
    Clock videoClock;

    /// 视频流索引
    int videoStreamIndex;

    /// 前视频流索引
    int videoLastStreamIndex;

#if CONFIG_AVFILTER
    /// 视频过滤器索引
    int videoFilterIndex;

    /// 第一个视频过滤器
    /// the first filter in the video chain
    AVFilterContext *videoInFilter;

    /// 最后一个视频过滤器
    /// the last filter in the video chain
    AVFilterContext *videoOutFilter;
#endif


    ////////////////////////////////////////
    ////////////////////////////////////////
    ////////////////////////////////////////
    /// Audio

    /// 音频码流
    AVStream *audioStream;

    /// 音频帧队列
    FrameQueue audioFrameQueue;

    /// 音频包队列
    PacketQueue audioPacketQueue;

    /// 音频解码器
    Decoder audioDecoder;

    /// 音频时钟
    Clock audioClock;

    /// 音频时钟时间
    double audioClockTime;

    /// 上一次音频时钟时间
    double lastAudioClockTime;

    ///
    int audioLastStreamIndex;

    /// 音频流索引
    int audioStreamIndex;

    /// 音频时钟seek序列
    int audioClockSeekSerial;

    /// 音频音量
    int audioVolume;

    /// 音频是否静音
    int audioMuted;

    /// 音频转码器上下文
    SwrContext *audioSwrContext;

    /// 音频缓冲区
    uint8_t *audioBuf;

    /// 音频转换缓冲区
    uint8_t *audioConvertBuf;

    /// 硬件缓冲区大小
    int audioHardwareBufSize;

    /// 音频缓冲索引
    /// in bytes
    int audioBufIndex;

    /// 音频缓冲大小
    /// in bytes
    unsigned int audioBufSize;

    /// 音频转换缓冲区大小
    unsigned int audioConvertBufSize;

    /// 用于音频差分计算
    /// used for AV difference average computation
    double audioDiffCum;

    ///
    double audioDiffAvgCoef;

    /// 音频差分阈值
    double audioDiffThreshold;

    /// 音频平均差分数量
    int audioDiffAvgCount;

    /// 音频写入缓冲区大小
    int audioWriteBufSize;

    /// 音频原参数
    AudioParams audioSrc;

    /// 音频目标参数
    AudioParams audioTarget;

#if CONFIG_AVFILTER

    /// 音频过滤器原参数
    AudioParams audioFilterSrc;

    /// 第一个音频过滤器
    /// the first filter in the audio chain
    AVFilterContext *inAudioFilter;

    /// 最后一个音频过滤器
    /// the last filter in the audio chain
    AVFilterContext *outAudioFilter;

    /// 音频过滤器
    /// audio filter graph
    AVFilterGraph *audioGraph;
#endif
};

#endif //SPLAYER_CORE_VIDEOSTATE_H
