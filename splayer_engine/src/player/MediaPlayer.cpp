#include <player/MediaPlayer.h>

MediaPlayer::MediaPlayer() {
    messageCenter = new MessageCenter();
    messageCenter->startMsgQueue();
};

MediaPlayer::~MediaPlayer() {
    if (audioDevice != nullptr) {
        delete audioDevice;
        audioDevice = nullptr;
    }
    if (videoDevice != nullptr) {
        delete videoDevice;
        videoDevice = nullptr;
    }
    if (mediaSync != nullptr) {
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
    if (DEBUG) ALOGD(TAG, "create media player - start");
    mutex.lock();
    _create();
    mutex.unlock();
    if (DEBUG) ALOGD(TAG, "create media player - end");
    return SUCCESS;
}

int MediaPlayer::start() {
    if (DEBUG) ALOGD(TAG, "start media player - start");
    mutex.lock();
    _start();
    mutex.unlock();
    if (DEBUG) ALOGD(TAG, "start media player - end");
    return SUCCESS;
}

int MediaPlayer::pause() {
    if (DEBUG) ALOGD(TAG, "pause media player - start");
    mutex.lock();
    togglePause();
    notifyMsg(Msg::MSG_PAUSE);
    mutex.unlock();
    if (DEBUG) ALOGD(TAG, "pause media player - end");
    return SUCCESS;
}

int MediaPlayer::play() {
    if (DEBUG) ALOGD(TAG, "play media player - start");
    mutex.lock();
    togglePause();
    notifyMsg(Msg::MSG_PLAY);
    mutex.unlock();
    if (DEBUG) ALOGD(TAG, "play media player - end");
    return SUCCESS;
}

int MediaPlayer::stop() {
    if (DEBUG) ALOGD(TAG, "stop media player - start");
    mutex.lock();
    _stop();
    mutex.unlock();
    if (DEBUG) ALOGD(TAG, "stop media player - end");
    return SUCCESS;
}

int MediaPlayer::destroy() {
    if (DEBUG) ALOGD(TAG, "destroy media player - start");
    mutex.lock();
    _destroy();
    mutex.unlock();
    if (DEBUG) ALOGD(TAG, "destroy media player - end");
    return SUCCESS;
}

int MediaPlayer::setDataSource(const char *url, int64_t offset,
                               const char *headers) {
    if (DEBUG)
        ALOGD(TAG, "%s url = %s offset = %lld headers = %p", __func__, url, offset,
              headers);
    mutex.lock();
    _setDataSource(url, offset, headers);
    mutex.unlock();
    return SUCCESS;
}

int MediaPlayer::_create() {
    if (messageCenter != nullptr) {
        messageCenter->setMsgListener(messageListener);
    }

    // Player State
    playerState = new PlayerState();

    // Media Sync
    if (mediaSync == nullptr) {
        mediaSync = new MediaSync();
    }
    if (mediaSync->create() < 0) {
        if (DEBUG) ALOGE(TAG, "media stream create failure");
        notifyMsg(Msg::MSG_ERROR, ERROR);
        return ERROR;
    }
    mediaSync->setMessageCenter(messageCenter);
    mediaSync->setPlayerState(playerState);

    // Media Stream
    mediaStream = new Stream(this, playerState);
    mediaStream->setMessageCenter(messageCenter);
    mediaStream->setStreamListener(this);
    mediaStream->setMediaSync(mediaSync);
    if (mediaStream->create() < 0) {
        if (DEBUG) ALOGE(TAG, "media stream create failure");
        notifyMsg(Msg::MSG_ERROR, ERROR);
        return ERROR;
    }

    // Audio Device
    if (audioDevice != nullptr) {
        audioDevice->create();
        if (audioResampler != nullptr) {
            audioResampler->setPlayerState(playerState);
            audioResampler->setMediaSync(mediaSync);
            audioResampler->create();
        }
    }

    // Video Device
    if (videoDevice != nullptr) {
        videoDevice->setPlayerState(playerState);
        if (videoDevice->create()) {
            mediaSync->setVideoDevice(videoDevice);
        } else {
            if (DEBUG) ALOGE(TAG, "%s video device create failure", __func__);
        }
    }

    notifyMsg(Msg::MSG_CREATE);

    return SUCCESS;
}

int MediaPlayer::_start() {
    if (checkParams() < 0) {
        notifyMsg(Msg::MSG_ERROR, ERROR_PARAMS);
        return ERROR_PARAMS;
    }
    playerState->abortRequest = 0;
    playerState->pauseRequest = 0;
    if (DEBUG) ALOGD(TAG, "start media stream");
    mediaStream->start();
    notifyMsg(Msg::MSG_START);
    notifyMsg(Msg::MSG_MEDIA_STREAM_START);
    return SUCCESS;
}

int MediaPlayer::_stop() {
    if (playerState != nullptr) {
        playerState->abortRequest = 1;
    }

    if (mediaSync != nullptr) {
        if (DEBUG) ALOGD(TAG, "stop media sync");
        mediaSync->stop();
        notifyMsg(Msg::MSG_MEDIA_SYNC_STOP);
    }

    if (audioDevice != nullptr) {
        if (DEBUG) ALOGD(TAG, "stop audio device");
        audioDevice->stop();
        notifyMsg(Msg::MSG_AUDIO_DEVICE_STOP);
    }

    if (videoDevice != nullptr) {
        if (DEBUG) ALOGD(TAG, "stop video device");
        notifyMsg(Msg::MSG_VIDEO_DEVICE_STOP);
    }

    if (audioDecoder != nullptr) {
        if (DEBUG) ALOGD(TAG, "stop audio decoder");
        audioDecoder->stop();
        notifyMsg(Msg::MSG_AUDIO_DECODER_STOP);
        delete audioDecoder;
        audioDecoder = nullptr;
    }

    if (videoDecoder != nullptr) {
        if (DEBUG) ALOGD(TAG, "stop video decoder");
        videoDecoder->stop();
        notifyMsg(Msg::MSG_VIDEO_DECODER_STOP);
        delete videoDecoder;
        videoDecoder = nullptr;
    }

    if (mediaStream != nullptr) {
        if (DEBUG) ALOGD(TAG, "stop media stream");
        mediaStream->stop();
        notifyMsg(Msg::MSG_MEDIA_STREAM_STOP);
    }

    if (audioResampler != nullptr) {
        delete audioResampler;
        audioResampler = nullptr;
    }

    if (formatContext != nullptr) {
        formatContext = nullptr;
    }

    if (playerState != nullptr) {
        playerState->reset();
    }

    notifyMsg(Msg::MSG_STOP);
    return SUCCESS;
}

int MediaPlayer::_destroy() {
    _stop();

    if (videoDevice != nullptr) {
        videoDevice->setPlayerState(nullptr);
        if (mediaSync != nullptr) {
            mediaSync->setVideoDevice(nullptr);
        }
        videoDevice->destroy();
        videoDevice = nullptr;
    }

    if (audioDevice != nullptr) {
        if (audioResampler != nullptr) {
            audioResampler->destroy();
            delete audioResampler;
            audioResampler = nullptr;
        }
        audioDevice->destroy();
        audioDevice = nullptr;
    }

    if (mediaStream != nullptr) {
        mediaStream->setMessageCenter(nullptr);
        mediaStream->setStreamListener(nullptr);
        mediaStream->setMediaSync(nullptr);
        mediaStream->destroy();
        delete mediaStream;
        mediaStream = nullptr;
    }

    if (mediaSync != nullptr) {
        mediaSync->setMessageCenter(nullptr);
        mediaSync->setPlayerState(nullptr);
        mediaSync->destroy();
        mediaSync = nullptr;
    }

    if (playerState != nullptr) {
        delete playerState;
        playerState = nullptr;
    }

    notifyMsg(Msg::MSG_DESTROY);
    return SUCCESS;
}

int MediaPlayer::_setDataSource(const char *url, int64_t offset,
                                const char *headers) const {
    if (playerState != nullptr) {
        playerState->url = av_strdup(url);
        playerState->offset = offset;
        if (headers != nullptr) {
            playerState->headers = av_strdup(headers);
        }
        return SUCCESS;
    }
    return ERROR;
}

int MediaPlayer::seekTo(float increment) {
    if (DEBUG) ALOGD(TAG, "%s increment = %lf", __func__, increment);
    if (!playerState) {
        if (DEBUG) ALOGE(TAG, "%s player state is null", __func__);
        return ERROR;
    }
    if (!playerState->realTime && playerState->duration < 0) {
        return ERROR_DURATION;
    }
    if (playerState->seekRequest) {
        return ERROR_LAST_SEEK_REQUEST;
    }
    double pos;
    if (playerState->seekByBytes) {
        pos = -1;
        if (pos < 0 && playerState->videoIndex >= 0) {
            pos = videoDecoder->getFrameQueueLastPos();
        }
        if (pos < 0) {
            pos = avio_tell(playerState->formatContext->pb);
        }
        if (playerState->formatContext->bit_rate) {
            increment *= playerState->formatContext->bit_rate / 8.0;
        } else {
            increment *= 180000.0;
        }
        pos += increment;
        seek((int64_t) pos, (int64_t) increment, 1);
    } else {
        pos = mediaSync->getMasterClock();
        if (isnan(pos)) {
            pos = (double) playerState->seekPos / AV_TIME_BASE;
        }
        pos += increment;
        if (playerState->formatContext->start_time != AV_NOPTS_VALUE &&
            pos < playerState->formatContext->start_time / (double) AV_TIME_BASE) {
            pos = playerState->formatContext->start_time / (double) AV_TIME_BASE;
        }
        seek((int64_t) (pos * AV_TIME_BASE), (int64_t) (increment * AV_TIME_BASE), 0);
    }

    return SUCCESS;
}

void MediaPlayer::setLooping(int looping) {
    mutex.lock();
    playerState->loopTimes = looping;
    condition.signal();
    mutex.unlock();
}

void MediaPlayer::setVolume(float leftVolume, float rightVolume) {
    if (audioDevice) {
        audioDevice->setStereoVolume(leftVolume, rightVolume);
    }
}

void MediaPlayer::setMute(int mute) {
    mutex.lock();
    playerState->audioMute = mute;
    condition.signal();
    mutex.unlock();
}

void MediaPlayer::setRate(float rate) {
    mutex.lock();
    playerState->playbackRate = rate;
    condition.signal();
    mutex.unlock();
}

void MediaPlayer::setPitch(float pitch) {
    mutex.lock();
    playerState->playbackPitch = pitch;
    condition.signal();
    mutex.unlock();
}

int MediaPlayer::getRotate() {
    Mutex::Autolock lock(mutex);
    if (videoDecoder) {
        return videoDecoder->getRotate();
    }
    return 0;
}

int MediaPlayer::getVideoWidth() {
    Mutex::Autolock lock(mutex);
    if (videoDecoder) {
        return videoDecoder->getCodecContext()->width;
    }
    return 0;
}

int MediaPlayer::getVideoHeight() {
    Mutex::Autolock lock(mutex);
    if (videoDecoder) {
        return videoDecoder->getCodecContext()->height;
    }
    return 0;
}

long MediaPlayer::getCurrentPosition() {
    Mutex::Autolock lock(mutex);
    int64_t currentPosition = 0;
    // 处于定位
    if (playerState->seekRequest) {
        currentPosition = playerState->seekPos;
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
            pos = playerState->seekPos;
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
    Mutex::Autolock lock(mutex);
    // TODO
    return (long) 0;
}

int MediaPlayer::isPlaying() {
    Mutex::Autolock lock(mutex);
    return !playerState->abortRequest && !playerState->pauseRequest;
}

int MediaPlayer::isLooping() { return playerState->loopTimes; }

int MediaPlayer::getMetadata(AVDictionary **metadata) {
    if (!formatContext) {
        return -1;
    }
    // TODO getMetadata
    return SUCCESS;
}

void audioPCMQueueCallback(void *opaque, uint8_t *stream, int len) {
    MediaPlayer *mediaPlayer = (MediaPlayer *) opaque;
    mediaPlayer->pcmQueueCallback(stream, len);
}

void MediaPlayer::pcmQueueCallback(uint8_t *stream, int len) {
    if (!audioResampler) {
        memset(stream, 0, sizeof(len));
        return;
    }
    audioResampler->pcmQueueCallback(stream, len);
}

void MediaPlayer::setMessageListener(IMessageListener *messageListener) {
    if (messageListener) {
        this->messageListener = messageListener;
    } else {
        if (DEBUG) ALOGE(TAG, "%s message listener is null", __func__);
    }
}

void MediaPlayer::setAudioDevice(AudioDevice *audioDevice) {
    this->audioDevice = audioDevice;
    if (this->audioDevice) {
        // 初始化音频重采样器
        audioResampler = new AudioReSampler();
    } else {
        if (DEBUG) ALOGE(TAG, "%s audio device is null", __func__);
    }
}

void MediaPlayer::setMediaSync(MediaSync *mediaSync) {
    this->mediaSync = mediaSync;
    if (this->mediaSync) {
    } else {
        if (DEBUG) ALOGE(TAG, "%s media sync is null", __func__);
    }
}

void MediaPlayer::setVideoDevice(VideoDevice *videoDevice) {
    this->videoDevice = videoDevice;
    if (this->videoDevice) {
    } else {
        if (DEBUG) ALOGE(TAG, "%s video device is null", __func__);
    }
}

int MediaPlayer::togglePause() {
    if (mediaSync) {
        mediaSync->togglePause();
    }
    return SUCCESS;
}

int MediaPlayer::openDecoder(int streamIndex) {
    AVCodecContext *codecContext = nullptr;
    AVCodec *codec = nullptr;
    AVDictionary *opts = nullptr;
    AVDictionaryEntry *t = nullptr;
    char *forcedCodecName = nullptr;
    int ret = 0;

    // 判断流索引的合法性
    if (formatContext == nullptr || streamIndex < 0 ||
        streamIndex >= formatContext->nb_streams) {
        if (DEBUG) ALOGE(TAG, "%s illegal stream index", __func__);
        closeDecoder(streamIndex, nullptr, nullptr);
        notifyMsg(Msg::MSG_ERROR, ERROR_STREAM_INDEX, streamIndex);
        return ERROR_STREAM_INDEX;
    }

    // 创建解码上下文
    codecContext = avcodec_alloc_context3(nullptr);
    if (!codecContext) {
        if (DEBUG) ALOGE(TAG, "%s alloc codec context failure", __func__);
        closeDecoder(streamIndex, codecContext, opts);
        notifyMsg(Msg::MSG_ERROR, ERROR_NOT_MEMORY);
        return ERROR_NOT_MEMORY;
    }

    // 复制解码上下文参数
    ret = avcodec_parameters_to_context(
            codecContext, formatContext->streams[streamIndex]->codecpar);
    if (ret < 0) {
        if (DEBUG) ALOGE(TAG, "%s copy codec params to context failure", __func__);
        closeDecoder(streamIndex, nullptr, nullptr);
        notifyMsg(Msg::MSG_ERROR, ERROR_COPY_CODEC_PARAM_TO_CONTEXT);
        return ERROR_COPY_CODEC_PARAM_TO_CONTEXT;
    }

    // 设置时钟基准
    codecContext->pkt_timebase = formatContext->streams[streamIndex]->time_base;

    // 优先使用指定的解码器
    if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
        forcedCodecName = playerState->audioCodecName;
    } else if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
        forcedCodecName = playerState->videoCodecName;
    }

    // 如果指定了解码器，则查找指定解码器
    if (forcedCodecName) {
        codec = avcodec_find_decoder_by_name(forcedCodecName);
    }

    // 如果没有找到指定的解码器，则查找默认的解码器
    if (!codec) {
        if (forcedCodecName) {
            if (DEBUG)
                ALOGD(TAG, "%s No codec could be found with name '%s'", __func__,
                      forcedCodecName);
        }
        codec = avcodec_find_decoder(codecContext->codec_id);
    }

    // 判断是否成功得到解码器
    if (!codec) {
        if (DEBUG)
            ALOGE(TAG, "%s No codec could be found with id %d", __func__,
                  codecContext->codec_id);
        closeDecoder(streamIndex, nullptr, nullptr);
        notifyMsg(Msg::MSG_ERROR, ERROR_NOT_FOUND_DCODE);
        return ERROR_NOT_FOUND_DCODE;
    }

    // 设置解码器的Id
    codecContext->codec_id = codec->id;

    // 判断是否需要重新设置lowres的值
    int streamLowResolution = playerState->lowResolution;
    if (streamLowResolution > codec->max_lowres) {
        if (DEBUG)
            ALOGD(TAG,
                  "%s The maximum value for low Resolution supported by the decoder "
                  "is %d",
                  __func__, codec->max_lowres);
        streamLowResolution = codec->max_lowres;
    }
    codecContext->lowres = streamLowResolution;

#if FF_API_EMU_EDGE
    if (stream_lowres) {
      codecContext->flags |= CODEC_FLAG_EMU_EDGE;
    }
#endif
    if (playerState->fast) {
        codecContext->flags2 |= AV_CODEC_FLAG2_FAST;
    }

#if FF_API_EMU_EDGE
    if (codec->capabilities & AV_CODEC_CAP_DR1) {
      codecContext->flags |= CODEC_FLAG_EMU_EDGE;
    }
#endif
    opts = filterCodecOptions(playerState->codecOpts, codecContext->codec_id,
                              formatContext, formatContext->streams[streamIndex],
                              codec);
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
    if ((ret = avcodec_open2(codecContext, codec, &opts)) < 0) {
        if (DEBUG) ALOGE(TAG, "%s open codec failure", __func__);
        closeDecoder(streamIndex, nullptr, nullptr);
        notifyMsg(Msg::MSG_ERROR, ERROR_NOT_OPEN_DECODE);
        return ERROR_NOT_OPEN_DECODE;
    }

    if ((t = av_dict_get(opts, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
        if (DEBUG) ALOGE(TAG, "%s option %s not found", __func__, t->key);
        closeDecoder(streamIndex, nullptr, nullptr);
        notifyMsg(Msg::MSG_ERROR, ERROR_CODEC_OPTIONS);
        return ERROR_CODEC_OPTIONS;
    }

    playerState->eof = 0;

    // 根据解码器类型创建解码器
    formatContext->streams[streamIndex]->discard = AVDISCARD_DEFAULT;

    if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
        audioDecoder = new AudioDecoder(
                codecContext,
                formatContext->streams[streamIndex],
                streamIndex,
                playerState,
                mediaStream->getFlushPacket(),
                mediaStream->getWaitCondition());
        mediaStream->setAudioDecoder(audioDecoder);
    } else if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
        videoDecoder = new VideoDecoder(
                formatContext,
                codecContext,
                formatContext->streams[streamIndex],
                streamIndex,
                playerState,
                mediaStream->getFlushPacket(),
                mediaStream->getWaitCondition());
        mediaStream->setVideoDecoder(videoDecoder);
        playerState->attachmentRequest = 1;
    }

    return SUCCESS;
}

int MediaPlayer::openAudioDevice(int64_t wanted_channel_layout,
                                 int wanted_nb_channels,
                                 int wanted_sample_rate) {
    AudioDeviceSpec wanted_spec, spec;
    const int next_nb_channels[] = {0, 0, 1, 6, 2, 6, 4, 6};
    const int next_sample_rates[] = {0, 44100, 48000, 96000, 192000};
    int next_sample_rate_idx = FF_ARRAY_ELEMS(next_sample_rates) - 1;

    if (!wanted_channel_layout ||
        wanted_nb_channels !=
        av_get_channel_layout_nb_channels((uint64_t) wanted_channel_layout)) {
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
        wanted_channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
    }
    wanted_nb_channels =
            av_get_channel_layout_nb_channels((uint64_t) wanted_channel_layout);

    wanted_spec.channels = (uint8_t) wanted_nb_channels;
    wanted_spec.freq = wanted_sample_rate;

    // 校验采样率和声道数量合法性
    if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0) {
        if (DEBUG) ALOGE(TAG, "%s Invalid sample rate or channel count!", __func__);
        return ERROR_AUDIO_SPEC;
    }

    // 找到合适的采样率
    while (next_sample_rate_idx &&
           next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq) {
        next_sample_rate_idx--;
    }

    wanted_spec.format = AV_SAMPLE_FMT_S16;
    wanted_spec.samples = (uint16_t) FFMAX(
            AUDIO_MIN_BUFFER_SIZE, 2 << av_log2((unsigned int) wanted_spec.freq /
                                                AUDIO_MAX_CALLBACKS_PER_SEC));
    wanted_spec.callback = audioPCMQueueCallback;
    wanted_spec.userdata = this;

    // 打开音频设备
    while (audioDevice->open(&wanted_spec, &spec) < 0) {
        if (DEBUG)
            ALOGD(TAG, "%s failed to open audio device: (%d channels, %d Hz)!",
                  __func__, wanted_spec.channels, wanted_spec.freq);
        wanted_spec.channels =
                (uint8_t) next_nb_channels[FFMIN(7, wanted_spec.channels)];
        if (!wanted_spec.channels) {
            wanted_spec.freq = next_sample_rates[next_sample_rate_idx--];
            wanted_spec.channels = (uint8_t) wanted_nb_channels;
            if (!wanted_spec.freq) {
                if (DEBUG)
                    ALOGE(TAG, "%s No more combinations to try, audio open failed",
                          __func__);
                return ERROR_AUDIO_SPEC;
            }
        }
        wanted_channel_layout = av_get_default_channel_layout(wanted_spec.channels);
    }

    if (spec.format != AV_SAMPLE_FMT_S16) {
        if (DEBUG)
            ALOGE(TAG, "%s audio format %d is not supported!", __func__, spec.format);
        return ERROR_AUDIO_FORMAT;
    }

    if (spec.channels != wanted_spec.channels) {
        wanted_channel_layout = av_get_default_channel_layout(spec.channels);
        if (!wanted_channel_layout) {
            if (DEBUG)
                ALOGE(TAG, "%s channel count %d is not supported!", __func__,
                      spec.channels);
            return ERROR_AUDIO_CHANNEL_LAYOUT;
        }
    }

    // 设置需要重采样的参数
    audioResampler->setReSampleParams(&spec, wanted_channel_layout);

    return spec.size;
}

void MediaPlayer::onStartOpenStream() {
    if (DEBUG) ALOGD(TAG, __func__);
}

void MediaPlayer::onEndOpenStream(int videoIndex, int audioIndex) {
    if (DEBUG)
        ALOGD(TAG, "%s videoIndex = %d audioIndex = %d", __func__, videoIndex,
              audioIndex);

    int ret = 0;

    // 根据媒体流索引准备解码器

    // 准备视频解码器
    if (videoIndex >= 0 && videoDevice != nullptr) {
        playerState->videoIndex = videoIndex;
        if (openDecoder(videoIndex) < 0) {
            if (DEBUG) ALOGE(TAG, "%s failed to create video decoder", __func__);
            notifyMsg(Msg::MSG_ERROR, ERROR_CREATE_AUDIO_DECODER);
        }
    }

    // 准备音频解码器
    if (audioIndex >= 0 && audioDevice != nullptr) {
        playerState->audioIndex = audioIndex;
        if (openDecoder(audioIndex) < 0) {
            if (DEBUG) ALOGE(TAG, "%s failed to create audio decoder", __func__);
            notifyMsg(Msg::MSG_ERROR, ERROR_CREATE_VIDEO_DECODER);
        }
    }

    if (audioDecoder == nullptr && videoDecoder == nullptr) {
        if (DEBUG)
            ALOGE(TAG, "%s failed to create audio and video decoder", __func__);
        notifyMsg(Msg::MSG_ERROR, ERROR_CREATE_VIDEO_AUDIO_DECODER);
        return;
    }

    // 视频解码器开始解码
    if (videoDecoder != nullptr) {
        if (DEBUG) ALOGD(TAG, "start video decoder");
        videoDecoder->start();
        notifyMsg(Msg::MSG_VIDEO_DECODER_START);
    } else {
        if (playerState->syncType == AV_SYNC_VIDEO) {
            playerState->syncType = AV_SYNC_AUDIO;
            if (DEBUG) ALOGD(TAG, "%s change sync type to AV_SYNC_AUDIO", __func__);
        }
    }

    // 音频解码器开始解码
    if (audioDecoder != nullptr) {
        if (DEBUG) ALOGD(TAG, "start audio decoder");
        if (audioResampler != nullptr) {
            audioResampler->setAudioDecoder(audioDecoder);
        }
        audioDecoder->start();
        notifyMsg(Msg::MSG_AUDIO_DECODER_START);
    } else {
        if (playerState->syncType == AV_SYNC_AUDIO) {
            playerState->syncType = AV_SYNC_EXTERNAL;
            if (DEBUG)
                ALOGD(TAG, "%s change sync type to AV_SYNC_EXTERNAL", __func__);
        }
    }

    if (DEBUG)
        ALOGD(TAG, "%s sync type = %s", __func__, playerState->getSyncType());

    // 打开视频输出设备
    if (videoDevice != nullptr) {
        if (DEBUG) ALOGD(TAG, "start video device");
        notifyMsg(Msg::MSG_VIDEO_DEVICE_START);
    }

    // 打开音频输出设备
    if (audioDevice != nullptr && audioDecoder != nullptr) {
        AVCodecContext *codecContext = audioDecoder->getCodecContext();
        ret = openAudioDevice(codecContext->channel_layout, codecContext->channels,
                              codecContext->sample_rate);
        if (ret < 0) {
            if (DEBUG) ALOGE(TAG, "%s could not open audio device", __func__);
            notifyMsg(Msg::MSG_NOT_OPEN_AUDIO_DEVICE);
            // 如果音频设备打开失败，则调整时钟的同步类型
            if (playerState->syncType == AV_SYNC_AUDIO) {
                if (videoDecoder != nullptr) {
                    playerState->syncType = AV_SYNC_VIDEO;
                } else {
                    playerState->syncType = AV_SYNC_EXTERNAL;
                }
            }
        } else {
            // 启动音频输出设备
            if (DEBUG) ALOGD(TAG, "start audio device");
            audioDevice->start();
            notifyMsg(Msg::MSG_AUDIO_DEVICE_START);
        }
    }

    if (videoDecoder != nullptr) {
        if (playerState->syncType == AV_SYNC_AUDIO) {
            if (DEBUG) ALOGD(TAG, "%s change master clock to audio clock", __func__);
            videoDecoder->setMasterClock(mediaSync->getAudioClock());
        } else if (playerState->syncType == AV_SYNC_VIDEO) {
            if (DEBUG) ALOGD(TAG, "%s change master clock to video clock", __func__);
            videoDecoder->setMasterClock(mediaSync->getVideoClock());
        } else {
            if (DEBUG)
                ALOGD(TAG, "%s change master clock to external clock", __func__);
            videoDecoder->setMasterClock(mediaSync->getExternalClock());
        }
    }

    // 开始同步
    if (mediaSync != nullptr) {
        if (DEBUG) ALOGD(TAG, "start media sync");
        mediaSync->start(videoDecoder, audioDecoder);
        notifyMsg(Msg::MSG_MEDIA_SYNC_START);
    }
}

int MediaPlayer::notifyMsg(int what) {
    if (messageCenter != nullptr) {
        messageCenter->notifyMsg(what);
        return SUCCESS;
    }
    return ERROR;
}

int MediaPlayer::notifyMsg(int what, int arg1) {
    if (messageCenter != nullptr) {
        messageCenter->notifyMsg(what, arg1);
        return SUCCESS;
    }
    return ERROR;
}

int MediaPlayer::notifyMsg(int what, int arg1, int arg2) {
    if (messageCenter != nullptr) {
        messageCenter->notifyMsg(what, arg1, arg2);
        return SUCCESS;
    }
    return ERROR;
}

int MediaPlayer::closeDecoder(int streamIndex, AVCodecContext *codecContext,
                              AVDictionary *opts) {
    if (DEBUG) ALOGD(TAG, "%s streamIndex = %d", __func__, streamIndex);
    // 准备失败，则需要释放创建的解码上下文
    avcodec_free_context(&codecContext);
    // 释放参数
    av_dict_free(&opts);
    return SUCCESS;
}

int MediaPlayer::checkParams() {
    if (!playerState->url) {
        if (DEBUG) ALOGE(TAG, "%s url is null", __func__);
        return ERROR_PARAMS;
    }
    return SUCCESS;
}

void MediaPlayer::setFormatContext(AVFormatContext *formatContext) {
    if (DEBUG) ALOGD(TAG, "%s format context = %p", __func__, formatContext);
    this->formatContext = formatContext;
}

int MediaPlayer::seek(int64_t pos, int64_t rel, int seekByBytes) {
    // * seek in the stream
    if (playerState && !playerState->seekRequest) {
        playerState->seekPos = pos;
        playerState->seekRel = rel;
        playerState->seekFlags &= ~AVSEEK_FLAG_BYTE;
        if (seekByBytes) {
            playerState->seekFlags |= AVSEEK_FLAG_BYTE;
        }
        playerState->seekRequest = true;
        if (mediaStream) {
            mediaStream->getWaitCondition()->signal();
        }
    }
    return SUCCESS;
}
