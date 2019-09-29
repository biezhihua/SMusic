#ifndef AUDIORESAMPLER_H
#define AUDIORESAMPLER_H

#include <player/PlayerState.h>
#include <sync/MediaSync.h>
#include <device/AudioDevice.h>
#include <SoundTouchWrapper.h>

/**
 * 音频参数
 */
typedef struct AudioParams {

    /// 采样率
    int sampleRate;

    /// 声道数
    int channels;

    /// 声道设计，单声道，双声道还是立体声
    int64_t channelLayout;

    /// 采样格式
    AVSampleFormat sampleFormat;

    /// 采样大小
    int frameSize;

    /// 每秒多少字节
    int bytesPerSec;
} AudioParams;

/**
 * 音频重采样状态结构体
 */
typedef struct AudioState {
    /// 音频时钟
    double audioClock;

    double audio_diff_cum;

    double audio_diff_avg_coef;

    double audio_diff_threshold;

    int audio_diff_avg_count;

    int audioHardwareBufSize;

    /// 输出缓冲大小
    uint8_t *outputBuffer = nullptr;

    /// 重采样大小
    uint8_t *reSampleBuffer = nullptr;

    /// SoundTouch缓冲
    short *soundTouchBuffer = nullptr;

    /// 缓冲大小
    unsigned int bufferSize;

    /// 重采样大小
    unsigned int reSampleSize;

    /// SoundTouch处理后的缓冲大小大小
    unsigned int soundTouchBufferSize;

    int bufferIndex;

    /// 写入大小
    int writeBufferSize;

    /// 音频转码上下文
    SwrContext *swrContext = nullptr;

    /// 音频回调时间
    int64_t audio_callback_time;

    /// 音频原始参数
    AudioParams audioParamsSrc;

    /// 音频目标参数
    AudioParams audioParamsTarget;

    int seekSerial = -1;
} AudioState;

/**
 * 音频重采样器
 */
class AudioReSampler {
    const char *const TAG = "AudioReSampler";

public:
    AudioReSampler();

    virtual ~AudioReSampler();

    int setReSampleParams(AudioDeviceSpec *spec, int64_t wanted_channel_layout);

    void pcmQueueCallback(uint8_t *stream, int len);

    virtual void create();

    virtual void destroy();

    void setPlayerState(PlayerState *playerState);

    void setMediaSync(MediaSync *mediaSync);

    void setAudioDecoder(AudioDecoder *audioDecoder);

private:

    int audioSynchronize(int nbSamples);

    int audioFrameReSample();

private:

    PlayerState *playerState = nullptr;

    MediaSync *mediaSync = nullptr;

    /// 音频解码器
    AudioDecoder *audioDecoder = nullptr;

    /// 音频重采样状态
    AudioState *audioState = nullptr;

    /// 变速变调处理
    SoundTouchWrapper *soundTouchWrapper = nullptr;

    uint64_t getWantedChannelLayout(Frame *frame) const;

    int convertAudio(int wantedNbSamples, Frame *frame) const;

    int initConvertSwrContext(int64_t wantedChannelLayout, Frame *frame) const;
};


#endif //AUDIORESAMPLER_H
