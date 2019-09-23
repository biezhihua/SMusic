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
    int freq;

    /// 声道数
    int channels;

    /// 声道设计，单声道，双声道还是立体声
    int64_t channel_layout;

    /// 采样格式
    enum AVSampleFormat fmt;

    /// 采样大小
    int frame_size;

    /// 每秒多少字节
    int bytes_per_sec;
} AudioParams;

/**
 * 音频重采样状态结构体
 */
typedef struct AudioState {
    double audioClock;                      // 音频时钟
    double audio_diff_cum;
    double audio_diff_avg_coef;
    double audio_diff_threshold;
    int audio_diff_avg_count;
    int audioHardwareBufSize;
    uint8_t *outputBuffer;                  // 输出缓冲大小
    uint8_t *reSampleBuffer;                // 重采样大小
    short *soundTouchBuffer;                // SoundTouch缓冲
    unsigned int bufferSize;                // 缓冲大小
    unsigned int reSampleSize;              // 重采样大小
    unsigned int soundTouchBufferSize;      // SoundTouch处理后的缓冲大小大小
    int bufferIndex;
    int writeBufferSize;                    // 写入大小
    SwrContext *swr_ctx;                    // 音频转码上下文
    int64_t audio_callback_time;            // 音频回调时间
    AudioParams audioParamsSrc;             // 音频原始参数
    AudioParams audioParamsTarget;          // 音频目标参数

    int audioSeekSerial;                    // 音频时钟seek序列
} AudioState;

/**
 * 音频重采样器
 */
class AudioReSampler {
    const char *const TAG = "AudioReSampler";

public:
    AudioReSampler(PlayerState *playerState, AudioDecoder *audioDecoder, MediaSync *mediaSync);

    virtual ~AudioReSampler();

    int setReSampleParams(AudioDeviceSpec *spec, int64_t wanted_channel_layout);

    void pcmQueueCallback(uint8_t *stream, int len);

private:
    int audioSynchronize(int nbSamples);

    int audioFrameReSample();

private:
    PlayerState *playerState;
    MediaSync *mediaSync;

    AVFrame *frame;
    AudioDecoder *audioDecoder;             // 音频解码器
    AudioState *audioState;                 // 音频重采样状态
    SoundTouchWrapper *soundTouchWrapper;   // 变速变调处理
};


#endif //AUDIORESAMPLER_H
