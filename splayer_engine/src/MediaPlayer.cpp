#include "MediaPlayer.h"

void audioPCMQueueCallback(void *opaque, uint8_t *stream, int len) {
    MediaPlayer *mediaPlayer = (MediaPlayer *) opaque;
    mediaPlayer->pcmQueueCallback(stream, len);
}

MediaPlayer::MediaPlayer() {
    messageCenter = new MessageCenter(this, this);
    messageCenter->startMsgQueue();
    changeStatus(IDLED);
};

MediaPlayer::~MediaPlayer() {
    if (audioDevice) {
        delete audioDevice;
        audioDevice = nullptr;
    }
    if (videoDevice) {
        delete videoDevice;
        videoDevice = nullptr;
    }
    if (mediaSync) {
        delete mediaSync;
        mediaSync = nullptr;
    }
    if (messageCenter) {
        messageCenter->stopMsgQueue();
        delete messageCenter;
        messageCenter = nullptr;
    }
}

int MediaPlayer::create() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    if (isIDLED()) {
        return syncCreate();
    } else {
        notExecuteWarning();
    }
    return ERROR;
}

int MediaPlayer::start() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    if (isCREATED() || isSTOPPED()) {
        notifyMsg(Msg::MSG_STATUS_PREPARE_START);
        return notifyMsg(Msg::MSG_REQUEST_START);
    } else {
        notExecuteWarning();
    };
    return ERROR;
}


int MediaPlayer::pause() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    if (isPLAYING()) {
        return notifyMsg(Msg::MSG_REQUEST_PAUSE);
    } else {
        notExecuteWarning();
    }
    return ERROR;
}

int MediaPlayer::play() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    if (isPAUSED()) {
        return notifyMsg(Msg::MSG_REQUEST_PLAY);
    } else {
        notExecuteWarning();
    }
    return ERROR;
}

int MediaPlayer::stop() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    return notifyMsg(Msg::MSG_REQUEST_STOP);
}

int MediaPlayer::destroy() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    if (isCREATED() || isSTARTED() || isPLAYING() || isPAUSED() || isSTOPPED() || isERRORED()) {
        return syncDestroy();
    } else {
        notExecuteWarning();
    }
    return SUCCESS;
}

int MediaPlayer::setDataSource(const char *url, int64_t offset, const char *headers) {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s] url = %s offset = %lld headers = %p", __func__, url, offset, headers);
    }
    return syncSetDataSource(url, offset, headers);
}

int MediaPlayer::seekTo(float increment) {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s] increment = %lf", __func__, increment);
    }
    notifyMsg(Msg::MSG_SEEK_START);
    return notifyMsg(Msg::MSG_REQUEST_SEEK, increment);
}

void MediaPlayer::setLooping(int looping) {
    mutex.lock();
    if (playerInfoStatus) {
        playerInfoStatus->loopTimes = looping;
    }
    condition.signal();
    mutex.unlock();
}

void MediaPlayer::setVolume(float leftVolume, float rightVolume) {
    if (audioDevice) {
        audioDevice->setStereoVolume(leftVolume, rightVolume);
    }
}

void MediaPlayer::setMute(int mute) {
    if (playerInfoStatus) {
        playerInfoStatus->audioMute = mute;
    }
}

void MediaPlayer::setRate(float rate) {
    if (playerInfoStatus) {
        playerInfoStatus->playbackRate = rate;
    }
}

void MediaPlayer::setPitch(float pitch) {
    if (playerInfoStatus) {
        playerInfoStatus->playbackPitch = pitch;
    }
}

int MediaPlayer::getRotate() {
    if (videoDecoder) {
        return videoDecoder->getRotate();
    }
    return 0;
}

int MediaPlayer::getVideoWidth() {
    if (videoDecoder) {
        return videoDecoder->getCodecContext()->width;
    }
    return 0;
}

int MediaPlayer::getVideoHeight() {
    if (videoDecoder) {
        return videoDecoder->getCodecContext()->height;
    }
    return 0;
}

long MediaPlayer::getCurrentPosition() {
    int64_t currentPosition = 0;
    // 处于定位
    if (playerInfoStatus->seekRequest) {
        currentPosition = playerInfoStatus->seekPos;
    } else {
        // 起始延时
        int64_t start_time = formatContext->start_time;
        int64_t start_diff = 0;
        if (start_time > 0 && start_time != AV_NOPTS_VALUE) {
            start_diff = av_rescale(start_time, 1000, AV_TIME_BASE);
        }

        // 计算主时钟的时间
        int64_t pos = 0;
        double clock = mediaSync->getMasterClock();
        if (isnan(clock)) {
            pos = playerInfoStatus->seekPos;
        } else {
            pos = (int64_t) (clock * 1000);
        }
        if (pos < 0 || pos < start_diff) {
            return 0;
        }
        return (long) (pos - start_diff);
    }
    return (long) currentPosition;
}

long MediaPlayer::getDuration() {
    long ret = 0;
    if (playerInfoStatus) {
        ret = (long) (playerInfoStatus->duration);
    }
    return ret;
}

bool MediaPlayer::isPlaying() {
    bool ret = false;
    if (playerInfoStatus) {
        ret = isPLAYING() && !playerInfoStatus->abortRequest && !playerInfoStatus->pauseRequest;
    }
    return ret;
}

int MediaPlayer::isLooping() {
    return playerInfoStatus->loopTimes;
}

int MediaPlayer::getMetadata(AVDictionary **metadata) {
    if (!formatContext) {
        return -1;
    }
    return SUCCESS;
}

void MediaPlayer::pcmQueueCallback(uint8_t *stream, int len) {
    if (!audioResample) {
        memset(stream, 0, sizeof(len));
        return;
    }
    audioResample->onPCMDataCallback(stream, len);
}

void MediaPlayer::setMessageListener(IMessageListener *messageListener) {
    if (messageListener) {
        this->messageListener = messageListener;
    } else {
        ALOGE(TAG, "[%s] message listener is null", __func__);
    }
}

void MediaPlayer::setAudioDevice(AudioDevice *audioDevice) {
    this->audioDevice = audioDevice;
    if (this->audioDevice) {
    } else {
        ALOGE(TAG, "[%s] audio device is null", __func__);
    }
}

void MediaPlayer::setMediaSync(MediaSync *mediaSync) {
    this->mediaSync = mediaSync;
    if (this->mediaSync) {
    } else {
        ALOGE(TAG, "[%s] media sync is null", __func__);
    }
}

void MediaPlayer::setVideoDevice(VideoDevice *videoDevice) {
    this->videoDevice = videoDevice;
    if (this->videoDevice) {
    } else {
        ALOGE(TAG, "[%s] video device is null", __func__);
    }
}

int MediaPlayer::openDecoder(int streamIndex) {
    AVCodecContext *codecContext = nullptr;
    AVCodec *codec = nullptr;
    AVDictionary *opts = nullptr;
    AVDictionaryEntry *t = nullptr;
    char *forcedCodecName = nullptr;
    int ret = 0;

    // 判断流索引的合法性
    if (!formatContext || streamIndex < 0 || streamIndex >= formatContext->nb_streams) {
        ALOGE(TAG, "[%s] illegal stream index", __func__);
        return ERROR_STREAM_INDEX;
    }

    // 创建解码上下文
    codecContext = avcodec_alloc_context3(nullptr);
    if (!codecContext) {
        ALOGE(TAG, "[%s] alloc codec nmpReference failure", __func__);
        return ERROR_NOT_MEMORY;
    }

    // 复制解码上下文参数
    ret = avcodec_parameters_to_context(codecContext,
                                        formatContext->streams[streamIndex]->codecpar);
    if (ret < 0) {
        ALOGE(TAG, "[%s] copy codec params to nmpReference failure", __func__);
        return ERROR_COPY_CODEC_PARAM_TO_CONTEXT;
    }

    // 设置时钟基准
    codecContext->pkt_timebase = formatContext->streams[streamIndex]->time_base;

    // 优先使用指定的解码器
    if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
        forcedCodecName = playerInfoStatus->audioCodecName;
    } else if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
        forcedCodecName = playerInfoStatus->videoCodecName;
    }

    // 如果指定了解码器，则查找指定解码器
    if (forcedCodecName) {
        codec = avcodec_find_decoder_by_name(forcedCodecName);
    }

    // 如果没有找到指定的解码器，则查找默认的解码器
    if (!codec) {
        if (forcedCodecName) {
            if (ENGINE_DEBUG) {
                ALOGD(TAG, "[%s] No codec could be found with name forcedCodecName=%s", __func__,
                      forcedCodecName);
            }
        }
        codec = avcodec_find_decoder(codecContext->codec_id);
    }

    // 判断是否成功得到解码器
    if (!codec) {
        ALOGE(TAG, "[%s] No codec could be found with id index=%d codec_id=%d", __func__,
              streamIndex, codecContext->codec_id);
        return ERROR_NOT_FOUND_DCODE;
    }

    // 设置解码器的Id
    codecContext->codec_id = codec->id;

    // 判断是否需要重新设置lowres的值
    int streamLowResolution = playerInfoStatus->lowResolution;
    if (streamLowResolution > codec->max_lowres) {
        if (ENGINE_DEBUG) {
            ALOGD(TAG, "[%s] The maximum value for low Resolution supported by the decoder is %d",
                  __func__, codec->max_lowres);
        }
        streamLowResolution = codec->max_lowres;
    }
    codecContext->lowres = streamLowResolution;

#if FF_API_EMU_EDGE
    if (stream_lowres) {
      codecContext->flags |= CODEC_FLAG_EMU_EDGE;
    }
#endif

    if (playerInfoStatus->fast) {
        codecContext->flags2 |= AV_CODEC_FLAG2_FAST;
    }

#if FF_API_EMU_EDGE
    if (codec->capabilities & AV_CODEC_CAP_DR1) {
      codecContext->flags |= CODEC_FLAG_EMU_EDGE;
    }
#endif

    opts = filterCodecOptions(playerInfoStatus->codecOpts, codecContext->codec_id, formatContext,
                              formatContext->streams[streamIndex], codec);
    if (!av_dict_get(opts, OPT_THREADS, nullptr, 0)) {
        av_dict_set(&opts, OPT_THREADS, "auto", 0);
    }

    if (streamLowResolution) {
        av_dict_set_int(&opts, OPT_LOW_RESOLUTION, streamLowResolution, 0);
    }

    if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO ||
        codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
        av_dict_set(&opts, OPT_REF_COUNTED_FRAMES, "1", 0);
    }

    // 打开解码器
    if (avcodec_open2(codecContext, codec, &opts) < 0) {
        ALOGE(TAG, "[%s] open codec failure", __func__);
        return ERROR_NOT_OPEN_DECODE;
    }

    if ((t = av_dict_get(opts, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
        ALOGE(TAG, "[%s] option %s not found", __func__, t->key);
        return ERROR_CODEC_OPTIONS;
    }

    playerInfoStatus->eof = 0;

    // 根据解码器类型创建解码器
    formatContext->streams[streamIndex]->discard = AVDISCARD_DEFAULT;

    if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
        audioDecoder = new AudioDecoder(formatContext, codecContext,
                                        formatContext->streams[streamIndex], streamIndex,
                                        playerInfoStatus,
                                        mediaStream->getFlushPacket(),
                                        mediaStream->getWaitCondition(),
                                        opts, messageCenter);
        mediaStream->setAudioDecoder(audioDecoder);
    } else if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
        videoDecoder = new VideoDecoder(formatContext, codecContext,
                                        formatContext->streams[streamIndex], streamIndex,
                                        playerInfoStatus,
                                        mediaStream->getFlushPacket(),
                                        mediaStream->getWaitCondition(), opts,
                                        messageCenter);
        mediaStream->setVideoDecoder(videoDecoder);
        playerInfoStatus->attachmentRequest = 1;
    }

    return SUCCESS;
}

int MediaPlayer::openAudioDevice(int64_t wantedChannelLayout, int wantedNbChannels,
                                 int wantedSampleRate) {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s] wantedChannelLayout = %lld wantedNbChannels = %d wantedSampleRate = %d",
              __func__, wantedChannelLayout, wantedNbChannels, wantedSampleRate);
    }
    AudioDeviceSpec desired, obtained;

    const int nextNbChannels[] = {0, 0, 1, 6, 2, 6, 4, 6};
    const int nextSampleRates[] = {0, 44100, 48000, 96000, 192000};

    int nextSampleRateIdx = FF_ARRAY_ELEMS(nextSampleRates) - 1;

    if (!wantedChannelLayout ||
        wantedNbChannels != av_get_channel_layout_nb_channels((uint64_t) wantedChannelLayout)) {
        wantedChannelLayout = av_get_default_channel_layout(wantedNbChannels);
        wantedChannelLayout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
    }

    // 根据声道布局获取声道数量
    wantedNbChannels = av_get_channel_layout_nb_channels((uint64_t) wantedChannelLayout);

    desired.channels = (uint8_t) wantedNbChannels;

    desired.sampleRate = wantedSampleRate;

    // 校验采样率和声道数量合法性
    if (desired.sampleRate <= 0 || desired.channels <= 0) {
        ALOGE(TAG, "[%s] Invalid sample rate or channel count!", __func__);
        return ERROR_AUDIO_SPEC;
    }

    // 找到合适的采样率
    while (nextSampleRateIdx && nextSampleRates[nextSampleRateIdx] >= desired.sampleRate) {
        nextSampleRateIdx--;
    }

    desired.format = AV_SAMPLE_FMT_S16;
    unsigned int audioBufferSize = (unsigned int) desired.sampleRate / AUDIO_MAX_CALLBACKS_PER_SEC;
    desired.samples = (uint16_t) FFMAX(AUDIO_MIN_BUFFER_SIZE, 2 << av_log2(audioBufferSize));
    desired.callback = audioPCMQueueCallback;
    desired.userdata = this;

    // 打开音频设备
    while (audioDevice->open(&desired, &obtained) < 0) {
        if (ENGINE_DEBUG) {
            ALOGD(TAG, "[%s] failed to open audio device: (%d channels, %d Hz)!", __func__,
                  desired.channels, desired.sampleRate);
        }
        desired.channels = (uint8_t) nextNbChannels[FFMIN(7, desired.channels)];
        if (!desired.channels) {
            desired.sampleRate = nextSampleRates[nextSampleRateIdx--];
            desired.channels = (uint8_t) wantedNbChannels;
            if (!desired.sampleRate) {
                ALOGE(TAG, "[%s] No more combinations to try, audio open failed", __func__);
                return ERROR_AUDIO_SPEC;
            }
        }
        wantedChannelLayout = av_get_default_channel_layout(desired.channels);
    }

    if (obtained.format != AV_SAMPLE_FMT_S16) {
        ALOGE(TAG, "[%s] audio format %d is not supported!", __func__, obtained.format);
        return ERROR_AUDIO_FORMAT;
    }

    if (obtained.channels != desired.channels) {
        wantedChannelLayout = av_get_default_channel_layout(obtained.channels);
        if (!wantedChannelLayout) {
            ALOGE(TAG, "[%s] channel count %d is not supported!", __func__, obtained.channels);
            return ERROR_AUDIO_CHANNEL_LAYOUT;
        }
    }

    // 设置需要重采样的参数
    audioResample->setReSampleParams(&obtained, wantedChannelLayout);

    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s] "
                   "channels = %d "
                   "format = %s "
                   "sample = %d "
                   "sampleRate = %d "
                   "size = %d",
              __func__,
              obtained.channels,
              av_get_sample_fmt_name((AVSampleFormat) obtained.format),
              obtained.samples,
              obtained.sampleRate,
              obtained.size
        );
    }

    return obtained.size;
}

int MediaPlayer::onStartOpenStream() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    return SUCCESS;
}

int MediaPlayer::onEndOpenStream(int videoIndex, int audioIndex) {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s] videoIndex = %d audioIndex = %d", __func__, videoIndex, audioIndex);
    }

    // 根据媒体流索引准备解码器

    // 准备视频解码器
    if (videoIndex >= 0 && videoDevice) {
        playerInfoStatus->videoIndex = videoIndex;
        if (openDecoder(videoIndex) < 0) {
            ALOGE(TAG, "[%s] failed to init video decoder", __func__);
            return ERROR_CREATE_VIDEO_DECODER;
        }
    }

    // 准备音频解码器
    if (audioIndex >= 0 && audioDevice) {
        playerInfoStatus->audioIndex = audioIndex;
        if (openDecoder(audioIndex) < 0) {
            ALOGE(TAG, "[%s] failed to init audio decoder", __func__);
            if (videoDecoder == nullptr) {
                return ERROR_CREATE_AUDIO_DECODER;
            } else {
                audioDecoder = nullptr;
            }
        }
    }

    if (!audioDecoder && !videoDecoder) {
        ALOGE(TAG, "[%s] failed to init audio and video decoder", __func__);
        return ERROR_CREATE_VIDEO_AUDIO_DECODER;
    }

    // 准备解码器回调
    notifyMsg(Msg::MSG_PREPARED_DECODER);

    // 视频解码器开始解码
    if (videoDecoder) {
        if (ENGINE_DEBUG) {
            ALOGD(TAG, "[%s] start video decoder", __func__);
        }
        videoDecoder->start();
        notifyMsg(Msg::MSG_VIDEO_START);
    } else {
        if (playerInfoStatus->syncType == AV_SYNC_VIDEO) {
            playerInfoStatus->syncType = AV_SYNC_AUDIO;
            if (ENGINE_DEBUG) {
                ALOGD(TAG, "[%s] change sync type to AV_SYNC_AUDIO", __func__);
            }
        }
    }

    // 音频解码器开始解码
    if (audioDecoder) {
        if (ENGINE_DEBUG) {
            ALOGD(TAG, "[%s] start audio decoder", __func__);
        }
        if (audioResample) {
            audioResample->setAudioDecoder(audioDecoder);
        }
        audioDecoder->start();
        notifyMsg(Msg::MSG_AUDIO_START);
    } else {
        if (playerInfoStatus->syncType == AV_SYNC_AUDIO) {
            playerInfoStatus->syncType = AV_SYNC_EXTERNAL;
            if (ENGINE_DEBUG)
                ALOGD(TAG, "[%s] change sync type to AV_SYNC_EXTERNAL", __func__);
        }
    }

    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s] sync type = %s", __func__, playerInfoStatus->getSyncType());
    }

    // 打开视频输出设备
    if (videoDevice) {
        AVCodecParameters *codecpar = videoDecoder->getStream()->codecpar;
        notifyMsg(Msg::MSG_VIDEO_SIZE_CHANGED, codecpar->width, codecpar->height);
        notifyMsg(Msg::MSG_SAR_CHANGED, codecpar->sample_aspect_ratio.num,
                  codecpar->sample_aspect_ratio.den);
    }

    // 打开音频输出设备
    if (audioDevice && audioDecoder) {
        AVCodecContext *codecContext = audioDecoder->getCodecContext();
        if (openAudioDevice(codecContext->channel_layout, codecContext->channels,
                            codecContext->sample_rate) < 0) {
            ALOGE(TAG, "[%s] could not open audio device", __func__);
            notifyMsg(Msg::MSG_ERROR, ERROR_NOT_OPEN_AUDIO_DEVICE);
            // 如果音频设备打开失败，则调整时钟的同步类型
            if (playerInfoStatus->syncType == AV_SYNC_AUDIO) {
                if (videoDecoder) {
                    playerInfoStatus->syncType = AV_SYNC_VIDEO;
                } else {
                    playerInfoStatus->syncType = AV_SYNC_EXTERNAL;
                }
            }
        } else {
            // 启动音频输出设备
            audioDevice->start();
            notifyMsg(Msg::MSG_AUDIO_RENDERING_START);
        }
    }

    if (videoDecoder) {
        if (playerInfoStatus->syncType == AV_SYNC_AUDIO) {
            if (ENGINE_DEBUG) {
                ALOGD(TAG, "[%s] change master clock to audio clock", __func__);
            }
            videoDecoder->setMasterClock(mediaSync->getAudioClock());
        } else if (playerInfoStatus->syncType == AV_SYNC_VIDEO) {
            if (ENGINE_DEBUG) {
                ALOGD(TAG, "[%s] change master clock to video clock", __func__);
            }
            videoDecoder->setMasterClock(mediaSync->getVideoClock());
        } else {
            if (ENGINE_DEBUG) {
                ALOGD(TAG, "[%s] change master clock to external clock", __func__);
            }
            videoDecoder->setMasterClock(mediaSync->getExternalClock());
        }
    }

    // 开始同步
    if (mediaSync) {
        mediaSync->start(videoDecoder, audioDecoder);
        notifyMsg(Msg::MSG_VIDEO_ROTATION_CHANGED);
    } else {
        ALOGE(TAG, "[%s] media sync is null", __func__);
    }

    return SUCCESS;
}

int MediaPlayer::notifyMsg(int what) {
    if (messageCenter) {
        messageCenter->notifyMsg(what);
        return SUCCESS;
    }
    return ERROR;
}

int MediaPlayer::notifyMsg(int what, int arg1) {
    if (messageCenter) {
        messageCenter->notifyMsg(what, arg1);
        return SUCCESS;
    }
    return ERROR;
}

int MediaPlayer::notifyMsg(int what, float arg1) {
    if (messageCenter) {
        messageCenter->notifyMsg(what, arg1);
        return SUCCESS;
    }
    return ERROR;
}


int MediaPlayer::notifyMsg(int what, int arg1, int arg2) {
    if (messageCenter) {
        messageCenter->notifyMsg(what, arg1, arg2);
        return SUCCESS;
    }
    return ERROR;
}

int MediaPlayer::checkParams() {
    if (!playerInfoStatus->url) {
        ALOGE(TAG, "[%s] url is null", __func__);
        return ERROR_PARAMS;
    }
    return SUCCESS;
}

void MediaPlayer::setFormatContext(AVFormatContext *formatContext) {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s] format nmpReference = %p", __func__, formatContext);
    }
    this->formatContext = formatContext;
}

int MediaPlayer::syncCreate() {
    if (messageCenter) {
        messageCenter->setMsgListener(messageListener);
    }

    // Player State
    playerInfoStatus = new PlayerInfoStatus();

    // Media Sync
    if (!mediaSync) {
        mediaSync = new MediaSync();
    }
    if (mediaSync->create() < 0) {
        ALOGE(TAG, "[%s] media stream init failure", __func__);
        changeStatus(ERRORED);
        notifyMsg(Msg::MSG_STATUS_ERRORED);
        notifyMsg(Msg::MSG_ERROR, ERROR);
        return ERROR;
    }
    mediaSync->setMutex(&mutex);
    mediaSync->setCondition(&condition);
    mediaSync->setMessageCenter(messageCenter);
    mediaSync->setPlayerInfoStatus(playerInfoStatus);

    // Media Stream
    mediaStream = new Stream(this, playerInfoStatus);
    mediaStream->setMessageCenter(messageCenter);
    mediaStream->setStreamListener(this);
    mediaStream->setMediaSync(mediaSync);
    if (mediaStream->create() < 0) {
        ALOGE(TAG, "[%s] media stream init failure", __func__);
        changeStatus(ERRORED);
        notifyMsg(Msg::MSG_STATUS_ERRORED);
        notifyMsg(Msg::MSG_ERROR, ERROR);
        return ERROR;
    }

    // Audio Device
    if (audioDevice) {
        if (audioDevice->create() < 0) {
            ALOGE(TAG, "[%s] init audio device failure", __func__);
        } else {
            // 初始化音频重采样器
            audioResample = new AudioResample();
            audioResample->setPlayerState(playerInfoStatus);
            audioResample->setMediaSync(mediaSync);
            if (audioResample->create() < 0) {
                ALOGE(TAG, "[%s] init audio resample failure", __func__);
            }
        }
    }

    // Video Device
    if (videoDevice) {
        videoDevice->setPlayerInfoStatus(playerInfoStatus);
        if (videoDevice->create() < 0) {
            ALOGE(TAG, "[%s] init video device failure", __func__);
        } else {
            mediaSync->setVideoDevice(videoDevice);
        }
    }

    changeStatus(CREATED);
    notifyMsg(Msg::MSG_STATUS_CREATED);

    return SUCCESS;
}

int MediaPlayer::syncStart() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    if (checkParams() < 0) {
        notifyMsg(Msg::MSG_ERROR, ERROR_PARAMS);
        ALOGE(TAG, "[%s] incorrect parameter", __func__);
        return ERROR_PARAMS;
    }
    playerInfoStatus->setAbortRequest(0);
    playerInfoStatus->setPauseRequest(0);
    mediaStream->start();
    return SUCCESS;
}

int MediaPlayer::syncTogglePause() {
    if (mediaSync) {
        return mediaSync->togglePause();
    }
    return SUCCESS;
}

int MediaPlayer::syncSeekTo(float increment) {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s] increment = %lf", __func__, increment);
    }
    if (!(isPlaying() || isPAUSED())) {
        notExecuteWarning();
        return ERROR;
    }
    if (!playerInfoStatus) {
        ALOGE(TAG, "[%s] player state is null", __func__);
        return ERROR;
    }
    if (!playerInfoStatus->realTime && playerInfoStatus->duration < 0) {
        return ERROR_DURATION;
    }
    if (playerInfoStatus->seekRequest) {
        return ERROR_LAST_SEEK_REQUEST;
    }
    double pos;
    if (playerInfoStatus->seekByBytes) {
        pos = -1;
        if (pos < 0 && playerInfoStatus->videoIndex >= 0) {
            pos = videoDecoder->getFrameQueueLastPos();
        }
        if (pos < 0) {
            pos = avio_tell(playerInfoStatus->formatContext->pb);
        }
        if (playerInfoStatus->formatContext->bit_rate) {
            increment *= playerInfoStatus->formatContext->bit_rate / 8.0;
        } else {
            increment *= 180000.0;
        }
        pos += increment;
        syncSeekTo((int64_t) pos, (int64_t) increment, 1);
    } else {
        pos = mediaSync->getMasterClock();
        if (isnan(pos)) {
            pos = (double) playerInfoStatus->seekPos / AV_TIME_BASE;
        }
        pos += increment;
        if (playerInfoStatus->formatContext->start_time != AV_NOPTS_VALUE &&
            pos < playerInfoStatus->formatContext->start_time / (double) AV_TIME_BASE) {
            pos = playerInfoStatus->formatContext->start_time / (double) AV_TIME_BASE;
        }
        syncSeekTo((int64_t) (pos * AV_TIME_BASE), (int64_t) (increment * AV_TIME_BASE), 0);
    }
    return SUCCESS;
}

int MediaPlayer::syncSeekTo(int64_t pos, int64_t rel, int seekByBytes) {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s] pos=%lld rel=%lld seekByBytes=%d", __func__, pos, rel, seekByBytes);
    }
    if (playerInfoStatus && !playerInfoStatus->seekRequest) {
        playerInfoStatus->seekPos = pos;
        playerInfoStatus->seekRel = rel;
        playerInfoStatus->seekFlags &= ~AVSEEK_FLAG_BYTE;
        if (seekByBytes) {
            playerInfoStatus->seekFlags |= AVSEEK_FLAG_BYTE;
        }
        playerInfoStatus->setSeekRequest(true);
        if (mediaStream) {
            mediaStream->getWaitCondition()->signal();
        }
    }
    return SUCCESS;
}

int MediaPlayer::syncStop() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }

    if (!(isSTARTED() || isPAUSED() || isPLAYING())) {
        notExecuteWarning();
        return ERROR;
    }

    notifyMsg(Msg::MSG_STATUS_PREPARE_STOP);

    if (playerInfoStatus) {
        playerInfoStatus->setAbortRequest(1);
    }

    if (mediaSync) {
        mediaSync->stop();
    }

    if (audioDevice) {
        audioDevice->stop();
    }

    if (videoDevice) {
        //
    }

    if (audioDecoder) {
        audioDecoder->stop();
        delete audioDecoder;
        audioDecoder = nullptr;
    }

    if (videoDecoder) {
        videoDecoder->stop();
        delete videoDecoder;
        videoDecoder = nullptr;
    }

    if (mediaStream) {
        mediaStream->stop();
    }

    if (formatContext) {
        formatContext = nullptr;
    }

    if (playerInfoStatus) {
        playerInfoStatus->reset();
    }

    changeStatus(STOPPED);
    notifyMsg(Msg::MSG_STATUS_STOPPED);

    return SUCCESS;
}

int MediaPlayer::syncDestroy() {
    syncStop();

    notifyMsg(Msg::MSG_STATUS_PREPARE_DESTROY);

    if (videoDevice) {
        videoDevice->setPlayerInfoStatus(nullptr);
        if (mediaSync) {
            mediaSync->setVideoDevice(nullptr);
        }
        videoDevice->destroy();
        videoDevice = nullptr;
    }

    if (audioDevice) {
        if (audioResample) {
            audioResample->destroy();
            delete audioResample;
            audioResample = nullptr;
        }
        audioDevice->destroy();
        audioDevice = nullptr;
    }

    if (mediaStream) {
        mediaStream->setMessageCenter(nullptr);
        mediaStream->setStreamListener(nullptr);
        mediaStream->setMediaSync(nullptr);
        mediaStream->destroy();
        delete mediaStream;
        mediaStream = nullptr;
    }

    if (mediaSync) {
        mediaSync->setMessageCenter(nullptr);
        mediaSync->setPlayerInfoStatus(nullptr);
        mediaSync->setMutex(nullptr);
        mediaSync->setCondition(nullptr);
        mediaSync->destroy();
        mediaSync = nullptr;
    }

    changeStatus(DESTROYED);
    notifyMsg(Msg::MSG_STATUS_DESTROYED);

    if (playerInfoStatus) {
        delete playerInfoStatus;
        playerInfoStatus = nullptr;
    }

    changeStatus(IDLED);
    notifyMsg(Msg::MSG_STATUS_IDLED);

    return SUCCESS;
}

int MediaPlayer::syncSetDataSource(const char *url, int64_t offset, const char *headers) const {
    if (playerInfoStatus) {
        playerInfoStatus->url = av_strdup(url);
        playerInfoStatus->offset = offset;
        if (headers) {
            playerInfoStatus->headers = av_strdup(headers);
        }
        return SUCCESS;
    }
    return ERROR;
}

void MediaPlayer::setOption(int category, const char *type, const char *option) {
    if (playerInfoStatus) {
        playerInfoStatus->setOption(category, type, option);
    }
}

void MediaPlayer::setOption(int category, const char *type, int64_t option) {
    if (playerInfoStatus) {
        playerInfoStatus->setOptionLong(category, type, option);
    }
}

int MediaPlayer::syncPause() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    if (isPlaying() && syncTogglePause()) {
        changeStatus(PAUSED);
        notifyMsg(Msg::MSG_STATUS_PAUSED);
        return SUCCESS;
    } else {
        if (ENGINE_DEBUG) {
            ALOGD(TAG, "[%s] not playing or toggle pause error", __func__);
        }
    }
    return ERROR;
}


int MediaPlayer::syncPlay() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    if (!isPlaying() && syncTogglePause()) {
        changeStatus(PLAYING);
        notifyMsg(Msg::MSG_STATUS_PLAYING);
        return SUCCESS;
    } else {
        if (ENGINE_DEBUG) {
            ALOGD(TAG, "[%s] not pause or toggle pause error", __func__);
        }
    }
    return ERROR;
}


int MediaPlayer::changeStatus(PlayerStatus state) {
    mutex.lock();
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s] target status = %s, source status = %s", __func__, getStatus(state),
              getStatus(playerStatus));
    }
    playerStatus = state;
    mutex.unlock();
    return SUCCESS;
}

bool MediaPlayer::isIDLED() const { return playerStatus == IDLED; }

bool MediaPlayer::isCREATED() const { return playerStatus == CREATED; }

bool MediaPlayer::isSTARTED() const {
    return playerStatus == STARTED;
}

bool MediaPlayer::isSTOPPED() const { return playerStatus == STOPPED; }

bool MediaPlayer::isPLAYING() const { return playerStatus == PLAYING; }

bool MediaPlayer::isPAUSED() const { return playerStatus == PAUSED; }

bool MediaPlayer::isERRORED() const { return playerStatus == ERRORED; }

void MediaPlayer::notExecuteWarning() const {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s] incorrect status, current status = %s", __func__, getStatus(playerStatus));
    }
}

const char *MediaPlayer::getStatus(PlayerStatus arg) const {
    if (arg == ERRORED) {
        return "ERRORED";
    } else if (arg == CREATED) {
        return "CREATED";
    } else if (arg == STARTED) {
        return "STARTED";
    } else if (arg == PLAYING) {
        return "PLAYING";
    } else if (arg == PAUSED) {
        return "PAUSED";
    } else if (arg == STOPPED) {
        return "STOPPED";
    } else if (arg == DESTROYED) {
        return "DESTROYED";
    } else if (arg == IDLED) {
        return "IDLED";
    }
    return "ERRORED";
}
