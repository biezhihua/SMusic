#include <player/MediaPlayer.h>

MediaPlayer::MediaPlayer() {
    messageCenter = new MessageCenter();
    messageCenter->startMsgQueue();
};

MediaPlayer::~MediaPlayer() {
    if (messageCenter) {
        messageCenter->stopMsgQueue();
        delete messageCenter;
        messageCenter = nullptr;
    }
}

int MediaPlayer::create() {
    ALOGD(TAG, "create media player - start");
    mutex.lock();
    _create();
    mutex.unlock();
    ALOGD(TAG, "create media player - end");
    return SUCCESS;
}

int MediaPlayer::start() {
    ALOGD(TAG, "start media player - start");
    mutex.lock();
    _start();
    mutex.unlock();
    ALOGD(TAG, "start media player - end");
    return SUCCESS;
}

int MediaPlayer::pause() {
    ALOGD(TAG, "pause media player - start");
    mutex.lock();
    togglePause();
    notifyMsg(Msg::MSG_PAUSE);
    mutex.unlock();
    ALOGD(TAG, "pause media player - end");
    return SUCCESS;
}

int MediaPlayer::play() {
    ALOGD(TAG, "play media player - start");
    mutex.lock();
    togglePause();
    notifyMsg(Msg::MSG_PLAY);
    mutex.unlock();
    ALOGD(TAG, "play media player - end");
    return SUCCESS;
}

int MediaPlayer::stop() {
    ALOGD(TAG, "stop media player - start");
    mutex.lock();
    _stop();
    mutex.unlock();
    ALOGD(TAG, "stop media player - end");
    return SUCCESS;
}

int MediaPlayer::destroy() {
    ALOGD(TAG, "destroy media player - start");
    mutex.lock();
    _destroy();
    mutex.unlock();
    ALOGD(TAG, "destroy media player - end");
    return SUCCESS;
}

int MediaPlayer::setDataSource(const char *url, int64_t offset, const char *headers) {
    ALOGD(TAG, "%s url = %s offset = %lld headers = %p", __func__, url, offset, headers);
    mutex.lock();
    _setDataSource(url, offset, headers);
    mutex.unlock();
    return SUCCESS;
}

int MediaPlayer::_create() {

    if (messageCenter) {
        messageCenter->setMsgListener(messageListener);
    }

    // Player State
    playerState = new PlayerState();

    // Media Sync
    if (mediaSync == nullptr) {
        mediaSync = new MediaSync();
    }
    if (mediaSync->create() < 0) {
        ALOGE(TAG, "media stream create failure");
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
        ALOGE(TAG, "media stream create failure");
        notifyMsg(Msg::MSG_ERROR, ERROR);
        return ERROR;
    }

    // Video Device
    if (videoDevice != nullptr) {
        videoDevice->setPlayerState(playerState);
        if (videoDevice->create()) {
            mediaSync->setVideoDevice(videoDevice);
        } else {
            ALOGE(TAG, "%s video device create failure", __func__);
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
    ALOGD(TAG, "start media stream");
    mediaStream->start();
    notifyMsg(Msg::MSG_START);
    notifyMsg(Msg::MSG_MEDIA_STREAM_START);
    return SUCCESS;
}

int MediaPlayer::_stop() {
    if (playerState) {
        playerState->abortRequest = 1;
    }

    if (mediaSync) {
        ALOGD(TAG, "stop media sync");
        mediaSync->stop();
        notifyMsg(Msg::MSG_MEDIA_SYNC_STOP);
    }

    if (audioDevice != nullptr) {
        ALOGD(TAG, "stop audio device");
        audioDevice->stop();
        notifyMsg(Msg::MSG_AUDIO_DEVICE_STOP);
    }

    if (videoDevice != nullptr) {
        ALOGD(TAG, "stop video device");
        notifyMsg(Msg::MSG_VIDEO_DEVICE_STOP);
    }

    if (audioDecoder != nullptr) {
        ALOGD(TAG, "stop audio decoder");
        audioDecoder->stop();
        notifyMsg(Msg::MSG_AUDIO_DECODER_STOP);
        delete audioDecoder;
        audioDecoder = nullptr;
    }

    if (videoDecoder != nullptr) {
        ALOGD(TAG, "stop video decoder");
        videoDecoder->stop();
        notifyMsg(Msg::MSG_VIDEO_DECODER_STOP);
        delete videoDecoder;
        videoDecoder = nullptr;
    }

    if (mediaStream) {
        ALOGD(TAG, "stop media stream");
        mediaStream->stop();
        notifyMsg(Msg::MSG_MEDIA_STREAM_STOP);
    }

    if (audioResampler) {
        delete audioResampler;
        audioResampler = nullptr;
    }

    if (formatContext != nullptr) {
        formatContext = nullptr;
    }

    if (playerState) {
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
        delete videoDevice;
        videoDevice = nullptr;
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
        delete mediaSync;
        mediaSync = nullptr;
    }

    if (playerState) {
        delete playerState;
        playerState = nullptr;
    }

    notifyMsg(Msg::MSG_DESTROY);
    return SUCCESS;
}

int MediaPlayer::_setDataSource(const char *url, int64_t offset, const char *headers) const {
    if (playerState) {
        playerState->url = av_strdup(url);
        playerState->offset = offset;
        if (headers) {
            playerState->headers = av_strdup(headers);
        }
        return SUCCESS;
    }
    return ERROR;
}

void MediaPlayer::seekTo(float timeMs) {
    ALOGD(TAG, __func__);
    // when is a live media stream, duration is -1
//    if (!playerState->realTime && duration < 0) {
//        return;
//    }

    // 等待上一次操作完成
    mutex.lock();
    while (playerState->seekRequest) {
        condition.wait(mutex);
    }
    mutex.unlock();

    if (!playerState->seekRequest) {
        int64_t start_time = 0;
        int64_t seek_pos = av_rescale(timeMs, AV_TIME_BASE, 1000);
        start_time = formatContext ? formatContext->start_time : 0;
        if (start_time > 0 && start_time != AV_NOPTS_VALUE) {
            seek_pos += start_time;
        }
        playerState->seekPos = seek_pos;
        playerState->seekRel = 0;
        playerState->seekFlags &= ~AVSEEK_FLAG_BYTE;
        playerState->seekRequest = 1;
        condition.signal();
    }
}

void MediaPlayer::setLooping(int looping) {
    mutex.lock();
    playerState->loop = looping;
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
    playerState->mute = mute;
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

int MediaPlayer::isLooping() {
    return playerState->loop;
}

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
//    if (playerState->msgQueue && playerState->syncType != AV_SYNC_VIDEO) {
//    }
}

void MediaPlayer::setMessageListener(IMessageListener *messageListener) {
    if (messageListener) {
        this->messageListener = messageListener;
    } else {
        ALOGE(TAG, "%s message listener is null", __func__);
    }
}

void MediaPlayer::setAudioDevice(AudioDevice *audioDevice) {
    this->audioDevice = audioDevice;
    if (this->audioDevice) {
    } else {
        ALOGE(TAG, "%s audio device is null", __func__);
    }
}

void MediaPlayer::setMediaSync(MediaSync *mediaSync) {
    this->mediaSync = mediaSync;
    if (this->mediaSync) {
    } else {
        ALOGE(TAG, "%s media sync is null", __func__);
    }
}

void MediaPlayer::setVideoDevice(VideoDevice *videoDevice) {
    this->videoDevice = videoDevice;
    if (this->videoDevice) {

    } else {
        ALOGE(TAG, "%s video device is null", __func__);
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
    if (formatContext == nullptr || streamIndex < 0 || streamIndex >= formatContext->nb_streams) {
        ALOGE(TAG, "%s illegal stream index", __func__);
        closeDecoder(streamIndex, nullptr, nullptr);
        notifyMsg(Msg::MSG_ERROR, ERROR_STREAM_INDEX, streamIndex);
        return ERROR_STREAM_INDEX;
    }

    // 创建解码上下文
    codecContext = avcodec_alloc_context3(nullptr);
    if (!codecContext) {
        ALOGE(TAG, "%s alloc codec context failure", __func__);
        closeDecoder(streamIndex, codecContext, opts);
        notifyMsg(Msg::MSG_ERROR, ERROR_NOT_MEMORY);
        return ERROR_NOT_MEMORY;
    }

    // 复制解码上下文参数
    ret = avcodec_parameters_to_context(codecContext, formatContext->streams[streamIndex]->codecpar);
    if (ret < 0) {
        ALOGE(TAG, "%s copy codec params to context failure", __func__);
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
            ALOGD(TAG, "%s No codec could be found with name '%s'", __func__, forcedCodecName);
        }
        codec = avcodec_find_decoder(codecContext->codec_id);
    }

    // 判断是否成功得到解码器
    if (!codec) {
        ALOGE(TAG, "%s No codec could be found with id %d", __func__, codecContext->codec_id);
        closeDecoder(streamIndex, nullptr, nullptr);
        notifyMsg(Msg::MSG_ERROR, ERROR_NOT_FOUND_DCODE);
        return ERROR_NOT_FOUND_DCODE;
    }

    // 设置解码器的Id
    codecContext->codec_id = codec->id;

    // 判断是否需要重新设置lowres的值
    int streamLowResolution = playerState->lowres;
    if (streamLowResolution > codec->max_lowres) {
        ALOGD(TAG, "%s The maximum value for low Resolution supported by the decoder is %d", __func__,
              codec->max_lowres);
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
    opts = filterCodecOptions(playerState->codec_opts, codecContext->codec_id, formatContext,
                              formatContext->streams[streamIndex], codec);
    if (!av_dict_get(opts, OPT_THREADS, nullptr, 0)) {
        av_dict_set(&opts, OPT_THREADS, "auto", 0);
    }

    if (streamLowResolution) {
        av_dict_set_int(&opts, OPT_LOW_RESOLUTION, streamLowResolution, 0);
    }

    if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO || codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
        av_dict_set(&opts, OPT_REF_COUNTED_FRAMES, "1", 0);
    }

    // 打开解码器
    if ((ret = avcodec_open2(codecContext, codec, &opts)) < 0) {
        ALOGE(TAG, "%s open codec failure", __func__);
        closeDecoder(streamIndex, nullptr, nullptr);
        notifyMsg(Msg::MSG_ERROR, ERROR_NOT_OPEN_DECODE);
        return ERROR_NOT_OPEN_DECODE;
    }

    if ((t = av_dict_get(opts, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
        ALOGE(TAG, "%s option %s not found", __func__, t->key);
        closeDecoder(streamIndex, nullptr, nullptr);
        notifyMsg(Msg::MSG_ERROR, ERROR_CODEC_OPTIONS);
        return ERROR_CODEC_OPTIONS;
    }

    playerState->eof = 0;

    // 根据解码器类型创建解码器
    formatContext->streams[streamIndex]->discard = AVDISCARD_DEFAULT;

    if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
        audioDecoder = new AudioDecoder(codecContext,
                                        formatContext->streams[streamIndex],
                                        streamIndex,
                                        playerState, mediaStream->getFlushPacket(),
                                        mediaStream->getWaitCondition());
        mediaStream->setAudioDecoder(audioDecoder);
    } else if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
        videoDecoder = new VideoDecoder(formatContext,
                                        codecContext,
                                        formatContext->streams[streamIndex],
                                        streamIndex,
                                        playerState, mediaStream->getFlushPacket(),
                                        mediaStream->getWaitCondition());
        mediaStream->setVideoDecoder(videoDecoder);
        playerState->attachmentRequest = 1;
    }

    return SUCCESS;
}

int MediaPlayer::openAudioDevice(int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate) {
    AudioDeviceSpec wanted_spec, spec;
    const int next_nb_channels[] = {0, 0, 1, 6, 2, 6, 4, 6};
    const int next_sample_rates[] = {0, 44100, 48000, 96000, 192000};
    int next_sample_rate_idx = FF_ARRAY_ELEMS(next_sample_rates) - 1;

    if (wanted_nb_channels != av_get_channel_layout_nb_channels(wanted_channel_layout) || !wanted_channel_layout) {
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
        wanted_channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
    }

    wanted_nb_channels = av_get_channel_layout_nb_channels(wanted_channel_layout);
    wanted_spec.channels = wanted_nb_channels;
    wanted_spec.freq = wanted_sample_rate;

    if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0) {
        av_log(nullptr, AV_LOG_ERROR, "Invalid sample rate or channel count!\n");
        return -1;
    }

    while (next_sample_rate_idx && next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq) {
        next_sample_rate_idx--;
    }

    wanted_spec.format = AV_SAMPLE_FMT_S16;
    wanted_spec.samples = FFMAX(AUDIO_MIN_BUFFER_SIZE, 2 << av_log2(wanted_spec.freq / AUDIO_MAX_CALLBACKS_PER_SEC));
    wanted_spec.callback = audioPCMQueueCallback;
    wanted_spec.userdata = this;

    // 打开音频设备
    while (audioDevice->open(&wanted_spec, &spec) < 0) {
        ALOGD(TAG, "%s Failed to open audio device: (%d channels, %d Hz)!", __func__,
              wanted_spec.channels, wanted_spec.freq);
        wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)];
        if (!wanted_spec.channels) {
            wanted_spec.freq = next_sample_rates[next_sample_rate_idx--];
            wanted_spec.channels = wanted_nb_channels;
            if (!wanted_spec.freq) {
                ALOGE(TAG, "%s No more combinations to try, audio open failed", __func__);
                return -1;
            }
        }
        wanted_channel_layout = av_get_default_channel_layout(wanted_spec.channels);
    }

    if (spec.format != AV_SAMPLE_FMT_S16) {
        ALOGE(TAG, "%s audio format %d is not supported!", __func__, spec.format);
        return -1;
    }

    if (spec.channels != wanted_spec.channels) {
        wanted_channel_layout = av_get_default_channel_layout(spec.channels);
        if (!wanted_channel_layout) {
            ALOGE(TAG, "%s channel count %d is not supported!", __func__, spec.channels);
            return -1;
        }
    }

    // 初始化音频重采样器
    if (!audioResampler) {
        audioResampler = new AudioResampler(playerState, audioDecoder, mediaSync);
    }
    // 设置需要重采样的参数
    audioResampler->setResampleParams(&spec, wanted_channel_layout);

    return spec.size;
}

void MediaPlayer::onStartOpenStream() {
    ALOGD(TAG, __func__);
}

void MediaPlayer::onEndOpenStream(int videoIndex, int audioIndex) {
    ALOGD(TAG, "%s videoIndex = %d audioIndex = %d", __func__, videoIndex, audioIndex);

    int ret = 0;

    // 根据媒体流索引准备解码器

    // 准备视频解码器
    if (videoIndex >= 0 && videoDevice != nullptr) {
        if (openDecoder(videoIndex) < 0) {
            ALOGE(TAG, "%s failed to create video decoder", __func__);
            notifyMsg(Msg::MSG_ERROR, ERROR_CREATE_AUDIO_DECODER);
        }
    }

    // 准备音频解码器
    if (audioIndex >= 0 && audioDevice != nullptr) {
        if (openDecoder(audioIndex) < 0) {
            ALOGE(TAG, "%s failed to create audio decoder", __func__);
            notifyMsg(Msg::MSG_ERROR, ERROR_CREATE_VIDEO_DECODER);
        }
    }

    if (!audioDecoder && !videoDecoder) {
        ALOGE(TAG, "%s failed to create audio and video decoder", __func__);
        notifyMsg(Msg::MSG_ERROR, ERROR_CREATE_VIDEO_AUDIO_DECODER);
        return;
    }

    // 视频解码器开始解码
    if (videoDecoder != nullptr) {
        ALOGD(TAG, "start video decoder");
        videoDecoder->start();
        notifyMsg(Msg::MSG_VIDEO_DECODER_START);
    } else {
        if (playerState->syncType == AV_SYNC_VIDEO) {
            playerState->syncType = AV_SYNC_AUDIO;
            ALOGD(TAG, "%s change sync type to AV_SYNC_AUDIO", __func__);
        }
    }

    // 音频解码器开始解码
    if (audioDecoder != nullptr) {
        ALOGD(TAG, "start audio decoder");
        audioDecoder->start();
        notifyMsg(Msg::MSG_AUDIO_DECODER_START);
    } else {
        if (playerState->syncType == AV_SYNC_AUDIO) {
            playerState->syncType = AV_SYNC_EXTERNAL;
            ALOGD(TAG, "%s change sync type to AV_SYNC_EXTERNAL", __func__);
        }
    }

    ALOGD(TAG, "%s sync type = %s", __func__, playerState->getSyncType());

    // 打开视频输出设备
    if (videoDevice != nullptr) {
        ALOGD(TAG, "start video device");
        notifyMsg(Msg::MSG_VIDEO_DEVICE_START);
    }

    // 打开音频输出设备
    if (audioDevice != nullptr && audioDecoder != nullptr) {
        AVCodecContext *codecContext = audioDecoder->getCodecContext();
        ret = openAudioDevice(codecContext->channel_layout,
                              codecContext->channels,
                              codecContext->sample_rate);
        if (ret < 0) {
            ALOGE(TAG, "%s could not open audio device", __func__);
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
            ALOGD(TAG, "start audio device");
            audioDevice->start();
            notifyMsg(Msg::MSG_AUDIO_DEVICE_START);
        }
    }

    if (videoDecoder) {
        if (playerState->syncType == AV_SYNC_AUDIO) {
            ALOGD(TAG, "%s change master clock to audio clock", __func__);
            videoDecoder->setMasterClock(mediaSync->getAudioClock());
        } else if (playerState->syncType == AV_SYNC_VIDEO) {
            ALOGD(TAG, "%s change master clock to video clock", __func__);
            videoDecoder->setMasterClock(mediaSync->getVideoClock());
        } else {
            ALOGD(TAG, "%s change master clock to external clock", __func__);
            videoDecoder->setMasterClock(mediaSync->getExternalClock());
        }
    }

    // 开始同步
    if (mediaSync) {
        ALOGD(TAG, "start media sync");
        mediaSync->start(videoDecoder, audioDecoder);
        notifyMsg(Msg::MSG_MEDIA_SYNC_START);
    }
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

int MediaPlayer::notifyMsg(int what, int arg1, int arg2) {
    if (messageCenter) {
        messageCenter->notifyMsg(what, arg1, arg2);
        return SUCCESS;
    }
    return ERROR;
}

int MediaPlayer::closeDecoder(int streamIndex, AVCodecContext *codecContext, AVDictionary *opts) {
    ALOGD(TAG, "%s streamIndex = %d", __func__, streamIndex);
    // 准备失败，则需要释放创建的解码上下文
    avcodec_free_context(&codecContext);
    // 释放参数
    av_dict_free(&opts);
    return SUCCESS;
}

int MediaPlayer::checkParams() {
    if (!playerState->url) {
        ALOGE(TAG, "%s url is null", __func__);
        return ERROR_PARAMS;
    }
    return SUCCESS;
}

void MediaPlayer::setFormatContext(AVFormatContext *formatContext) {
    ALOGD(TAG, "%s format context = %p", __func__, formatContext);
    this->formatContext = formatContext;
}


