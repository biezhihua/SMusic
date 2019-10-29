#ifndef SPLAYER_ANDROID_SLESAUDIODEVICE_H
#define SPLAYER_ANDROID_SLESAUDIODEVICE_H

#include <device/AudioDevice.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>
#include <SLES/OpenSLES_AndroidMetadata.h>
#include <pthread.h>

/// 最大缓冲区数量
#define OPENSLES_BUFFERS 4

/// 缓冲区长度(毫秒)
#define OPENSLES_BUFLEN  10

/**
 * OPENSLES
 * https://developer.android.com/ndk/guides/audio/opensl/getting-started
 */
class AndroidAudioDevice : public AudioDevice {

    const char *const TAG = "[MP][ANDROID][AudioDevice]";

public:
    AndroidAudioDevice();

    ~AndroidAudioDevice() override;

    int open(AudioDeviceSpec *desired, AudioDeviceSpec *obtained) override;

    int create() override;

    void destroy() override;

    void start() override;

    void stop() override;

    void pause() override;

    void resume() override;

    void flush() override;

    void setStereoVolume(float left_volume, float right_volume) override;

    void run() override;

    void setMute(bool mute) override;

private:

    /// 转换成SL采样率
    SLuint32 getSLSampleRate(int sampleRate);

    /// 获取SLES音量
    SLmillibel getAmplificationLevel(float volumeLevel);

private:

    /// 初始对象
    SLObjectItf slObject = nullptr;

    /// 引擎接口
    SLEngineItf slEngine = nullptr;

    /// 混音器
    SLObjectItf slOutputMixObject = nullptr;

    /// 播放器对象
    SLObjectItf slPlayerObject = nullptr;

    /// 播放对象
    SLPlayItf slPlayItf = nullptr;

    /// 声音对象
    SLVolumeItf slVolumeItf = nullptr;

    /// 缓冲器队列接口
    SLAndroidSimpleBufferQueueItf slBufferQueueItf = nullptr;

    /// 音频设备参数
    AudioDeviceSpec audioDeviceSpec;

    /// 一帧占多少字节
    int bytes_per_frame;

    /// 一个缓冲区时长占多少
    int milli_per_buffer;

    /// 一个缓冲区有多少帧
    int frames_per_buffer;

    /// 一个缓冲区的大小
    int bytes_per_buffer;

    /// 缓冲区
    uint8_t *buffer = nullptr;

    /// 缓冲区总大小
    size_t buffer_capacity;

    /// 音频播放线程
    Thread *audioThread = nullptr;

    /// 终止标志
    int abortRequest;

    /// 暂停标志
    int pauseRequest;

    /// 刷新标志
    int flushRequest;

    /// 更新音量
    bool updateVolume;

    /// 左音量
    float leftVolume;

    /// 右音量
    float rightVolume;

    Mutex mutex;

    Condition condition;
};


#endif
