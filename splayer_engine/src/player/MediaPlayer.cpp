#include "player/MediaPlayer.h"

MediaPlayer::MediaPlayer() {
    avformat_network_init();
    av_init_packet(&flushPacket);
    flushPacket.data = (uint8_t *) &flushPacket;
    playerState = new PlayerState();
    duration = -1;
    audioDecoder = nullptr;
    videoDecoder = nullptr;
    formatContext = nullptr;
    lastPaused = -1;
    attachmentRequest = 0;
    audioResampler = nullptr;
    readThread = nullptr;
    quit = true;
}

MediaPlayer::~MediaPlayer() {
    flushPacket.data = nullptr;
    av_packet_unref(&flushPacket);
    avformat_network_deinit();
}

int MediaPlayer::reset() {
    ALOGD(TAG, __func__);
    stop();
    if (mediaSync) {
        mediaSync->reset();
        delete mediaSync;
        mediaSync = nullptr;
    }
    if (audioDecoder != nullptr) {
        audioDecoder->stop();
        delete audioDecoder;
        audioDecoder = nullptr;
    }
    if (videoDecoder != nullptr) {
        videoDecoder->stop();
        delete videoDecoder;
        videoDecoder = nullptr;
    }
    if (audioDevice != nullptr) {
        audioDevice->stop();
        delete audioDevice;
        audioDevice = nullptr;
    }
    if (audioResampler) {
        delete audioResampler;
        audioResampler = nullptr;
    }
    if (formatContext != nullptr) {
        avformat_close_input(&formatContext);
        avformat_free_context(formatContext);
        formatContext = nullptr;
    }
    if (playerState) {
        delete playerState;
        playerState = nullptr;
    }
    return SUCCESS;
}

void MediaPlayer::setDataSource(const char *url, int64_t offset, const char *headers) {
    ALOGD(TAG, "%s url = %s offset = %lld headers = %p", __func__, url, offset, headers);
    Mutex::Autolock lock(mutex);
    playerState->url = av_strdup(url);
    playerState->offset = offset;
    if (headers) {
        playerState->headers = av_strdup(headers);
    }
}

int MediaPlayer::prepareAsync() {
    ALOGD(TAG, __func__);
    Mutex::Autolock lock(mutex);
    if (!playerState->url) {
        ALOGE(TAG, "%s url is null", __func__);
        return ERROR_PARAMS;
    }
    playerState->abortRequest = 0;
    if (!readThread) {
        readThread = new Thread(this);
        readThread->start();
    }
    return SUCCESS;
}

void MediaPlayer::start() {
    ALOGD(TAG, __func__);
    Mutex::Autolock lock(mutex);
    playerState->abortRequest = 0;
    playerState->pauseRequest = 0;
    quit = false;
    condition.signal();
}

void MediaPlayer::pause() {
    ALOGD(TAG, __func__);
    Mutex::Autolock lock(mutex);
    playerState->pauseRequest = 1;
    condition.signal();
}

void MediaPlayer::resume() {
    ALOGD(TAG, __func__);
    Mutex::Autolock lock(mutex);
    playerState->pauseRequest = 0;
    condition.signal();
}

void MediaPlayer::stop() {
    ALOGD(TAG, __func__);
    mutex.lock();
    playerState->abortRequest = 1;
    condition.signal();
    mutex.unlock();
    mutex.lock();
    while (!quit) {
        condition.wait(mutex);
    }
    mutex.unlock();
    if (readThread != nullptr) {
        readThread->join();
        delete readThread;
        readThread = nullptr;
    }
}

void MediaPlayer::seekTo(float timeMs) {
    ALOGD(TAG, __func__);
    // when is a live media stream, duration is -1
    if (!playerState->realTime && duration < 0) {
        return;
    }

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
    return (long) duration;
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

static int avFormatInterruptCb(void *ctx) {
    PlayerState *playerState = (PlayerState *) ctx;
    if (playerState->abortRequest) {
        return AVERROR_EOF;
    }
    return 0;
}

MessageQueue *MediaPlayer::getMessageQueue() {
    Mutex::Autolock lock(mutex);
    return playerState->msgQueue;
}

PlayerState *MediaPlayer::getPlayerState() {
    Mutex::Autolock lock(mutex);
    return playerState;
}

void MediaPlayer::run() {
    readPackets();
}

int MediaPlayer::readPackets() {
    ALOGD(TAG, __func__);

    AVDictionaryEntry *t;
    AVDictionary **opts;
    int scanAllProgramMapTableSet = 0;
    int ret = 0;

    // 准备解码器
    mutex.lock();
    do {
        // 创建解复用上下文
        formatContext = avformat_alloc_context();
        if (!formatContext) {
            ALOGE(TAG, "%s avformat could not allocate context", __func__);
            ret = ERROR_NOT_MEMORY;
            break;
        }

        // 设置解复用中断回调
        formatContext->interrupt_callback.callback = avFormatInterruptCb;
        formatContext->interrupt_callback.opaque = playerState;

        // https://zhuanlan.zhihu.com/p/43672062
        // scan_all_pmts, 是mpegts的一个选项，这里在没有设定该选项的时候，强制设为1
        // scan_all_pmts, 扫描全部的ts流的"Program Map Table"表。
        if (!av_dict_get(playerState->format_opts, OPT_SCALL_ALL_PMTS, nullptr, AV_DICT_MATCH_CASE)) {
            av_dict_set(&playerState->format_opts, OPT_SCALL_ALL_PMTS, "1", AV_DICT_DONT_OVERWRITE);
            scanAllProgramMapTableSet = 1;
        }

        // 处理文件头
        if (playerState->headers) {
            av_dict_set(&playerState->format_opts, OPT_HEADERS, playerState->headers, 0);
        }

        // 处理文件偏移量
        if (playerState->offset > 0) {
            formatContext->skip_initial_bytes = playerState->offset;
        }

        // 设置rtmp/rtsp的超时值
        if (av_stristart(playerState->url, "rtmp", nullptr) || av_stristart(playerState->url, "rtsp", nullptr)) {
            // There is total different meaning for 'timeout' option in rtmp
            ALOGD(TAG, "%s remove 'timeout' option for rtmp.", __func__);
            av_dict_set(&playerState->format_opts, "timeout", nullptr, 0);
        }

        // 打开文件
        ret = avformat_open_input(&formatContext, playerState->url, playerState->inputFormat,
                                  &playerState->format_opts);
        if (ret < 0) {
            ALOGE(TAG, "%s avformat could not open input", __func__);
            ret = ERROR_NOT_OPEN_INPUT;
            break;
        }

        // 还原MPEGTS的特殊处理标记
        if (scanAllProgramMapTableSet) {
            av_dict_set(&playerState->format_opts, OPT_SCALL_ALL_PMTS, nullptr, AV_DICT_MATCH_CASE);
        }

        if ((t = av_dict_get(playerState->format_opts, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
            ALOGE(TAG, "%s Option %s not found", __func__, t->key);
            ret = ERROR_CODEC_OPTIONS;
            break;
        }

        if (playerState->generateMissingPts) {
            formatContext->flags |= AVFMT_FLAG_GENPTS;
        }
        av_format_inject_global_side_data(formatContext);

        opts = setupStreamInfoOptions(formatContext, playerState->codec_opts);

        // 查找媒体流信息
        ret = avformat_find_stream_info(formatContext, opts);
        if (opts != nullptr) {
            for (int i = 0; i < formatContext->nb_streams; i++) {
                if (opts[i] != nullptr) {
                    av_dict_free(&opts[i]);
                }
            }
            av_freep(&opts);
        }

        if (ret < 0) {
            ALOGD(TAG, "%s %s: could not find codec parameters", __func__, playerState->url);
            ret = ERROR_NOT_FOUND_STREAM_INFO;
            break;
        }

        // 判断是否实时流，判断是否需要设置无限缓冲区
        playerState->realTime = isRealTime(formatContext);
        if (playerState->infiniteBuffer < 0 && playerState->realTime) {
            playerState->infiniteBuffer = 1;
        }

        // Gets the duration of the file, -1 if no duration available.
        if (playerState->realTime) {
            duration = -1;
        } else {
            duration = -1;
            if (formatContext->duration != AV_NOPTS_VALUE) {
                duration = av_rescale(formatContext->duration, 1000, AV_TIME_BASE);
            }
        }
        playerState->videoDuration = duration;

        // I/O context.
        if (formatContext->pb) {
            formatContext->pb->eof_reached = 0;
        }

        // 判断是否以字节方式定位
        playerState->seekByBytes = !!(formatContext->iformat->flags & AVFMT_TS_DISCONT) &&
                                   strcmp(FORMAT_OGG, formatContext->iformat->name) != 0;

        // 设置最大帧间隔
        mediaSync->setMaxDuration((formatContext->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0);

        // 如果不是从头开始播放，则跳转到播放位置
        if (playerState->startTime != AV_NOPTS_VALUE) {
            int64_t timestamp;
            timestamp = playerState->startTime;
            if (formatContext->start_time != AV_NOPTS_VALUE) {
                timestamp += formatContext->start_time;
            }
            playerState->mMutex.lock();
            ret = avformat_seek_file(formatContext, -1, INT64_MIN, timestamp, INT64_MAX, 0);
            playerState->mMutex.unlock();
            if (ret < 0) {
                ALOGE(TAG, "%s %s: could not seek to position %0.3f", __func__, playerState->url,
                      (double) timestamp / AV_TIME_BASE);
            }
        }

        // 查找媒体流信息
        int audioIndex = -1;
        int videoIndex = -1;
        for (int i = 0; i < formatContext->nb_streams; ++i) {
            AVStream *stream = formatContext->streams[i];
            AVMediaType type = stream->codecpar->codec_type;
            stream->discard = AVDISCARD_ALL;
            if (type == AVMEDIA_TYPE_AUDIO) {
                if (audioIndex == -1) {
                    audioIndex = i;
                }
            } else if (type == AVMEDIA_TYPE_VIDEO) {
                if (videoIndex == -1) {
                    videoIndex = i;
                }
            }
        }

        // 如果不禁止视频流，则查找最合适的视频流索引
        if (!playerState->videoDisable) {
            videoIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, videoIndex, -1, nullptr, 0);
        } else {
            videoIndex = -1;
        }

        // 如果不禁止音频流，则查找最合适的音频流索引(与视频流关联的音频流)
        if (!playerState->audioDisable) {
            audioIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, audioIndex, videoIndex, nullptr, 0);
        } else {
            audioIndex = -1;
        }

        // 如果音频流和视频流都没有找到，则直接退出
        if (audioIndex == -1 && videoIndex == -1) {
            ALOGE(TAG, "%s could not find audio and video stream", __func__);
            ret = ERROR_DISABLE_ALL_STREAM;
            break;
        }

        // 根据媒体流索引准备解码器

        if (audioIndex >= 0) {
            prepareDecoder(audioIndex);
        }

        if (videoIndex >= 0) {
            prepareDecoder(videoIndex);
        }

        if (!audioDecoder && !videoDecoder) {
            ALOGE(TAG, "%s failed to create audio and video decoder", __func__);
            ret = ERROR_CREATE_DECODER;
            break;
        }

        ret = 0;

    } while (false);
    mutex.unlock();

    // 出错返回
    if (ret < 0) {
        ALOGE(TAG, "%s init context failure", __func__);
        quit = true;
        condition.signal();
        return ret;
    }

    if (videoDecoder) {
        AVCodecParameters *codecpar = videoDecoder->getStream()->codecpar;
    }

    // 视频解码器开始解码
    if (videoDecoder != nullptr) {
        videoDecoder->start();
    } else {
        if (playerState->syncType == AV_SYNC_VIDEO) {
            playerState->syncType = AV_SYNC_AUDIO;
        }
    }

    // 音频解码器开始解码
    if (audioDecoder != nullptr) {
        audioDecoder->start();
    } else {
        if (playerState->syncType == AV_SYNC_AUDIO) {
            playerState->syncType = AV_SYNC_EXTERNAL;
        }
    }

    // 打开音频输出设备
    if (audioDecoder != nullptr) {
        AVCodecContext *codecContext = audioDecoder->getCodecContext();
        ret = openAudioDevice(codecContext->channel_layout, codecContext->channels, codecContext->sample_rate);
        if (ret < 0) {
            ALOGE(TAG, "%s could not open audio device", __func__);
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
            audioDevice->start();
        }
    }

    if (videoDecoder) {
        if (playerState->syncType == AV_SYNC_AUDIO) {
            videoDecoder->setMasterClock(mediaSync->getAudioClock());
        } else if (playerState->syncType == AV_SYNC_VIDEO) {
            videoDecoder->setMasterClock(mediaSync->getVideoClock());
        } else {
            videoDecoder->setMasterClock(mediaSync->getExternalClock());
        }
    }

    // 开始同步
    mediaSync->start(videoDecoder, audioDecoder);

    // 读数据包流程
    eof = 0;
    ret = 0;
    AVPacket pkt1, *pkt = &pkt1;
    for (;;) {

        // 退出播放器
        if (playerState->abortRequest) {
            break;
        }

        // 是否暂停
        if (playerState->pauseRequest != lastPaused) {
            lastPaused = playerState->pauseRequest;
            if (playerState->pauseRequest) {
                av_read_pause(formatContext);
            } else {
                av_read_play(formatContext);
            }
        }

        // 定位处理
        if (playerState->seekRequest) {
            int64_t seek_target = playerState->seekPos;
            int64_t seek_min = playerState->seekRel > 0 ? seek_target - playerState->seekRel + 2 : INT64_MIN;
            int64_t seek_max = playerState->seekRel < 0 ? seek_target - playerState->seekRel - 2 : INT64_MAX;
            // 定位
            playerState->mMutex.lock();
            ret = avformat_seek_file(formatContext, -1, seek_min, seek_target, seek_max, playerState->seekFlags);
            playerState->mMutex.unlock();
            if (ret < 0) {
                ALOGD(TAG, "%s %s: error while seeking", __func__, playerState->url);
            } else {
                if (audioDecoder) {
                    audioDecoder->flush();
                    audioDecoder->pushFlushPacket();
                    ALOGD(TAG, "%s flush audio", __func__);
                }
                if (videoDecoder) {
                    videoDecoder->flush();
                    videoDecoder->pushFlushPacket();
                    ALOGD(TAG, "%s flush video", __func__);
                }
                // 更新外部时钟值
                if (playerState->seekFlags & AVSEEK_FLAG_BYTE) {
                    mediaSync->updateExternalClock(NAN, 0);
                } else {
                    mediaSync->updateExternalClock(seek_target / (double) AV_TIME_BASE, 0);
                }
                mediaSync->refreshVideoTimer();
            }
            attachmentRequest = 1;
            playerState->seekRequest = 0;
            condition.signal();
            eof = 0;
        }

        // 取得封面数据包
        if (attachmentRequest) {
            // https://segmentfault.com/a/1190000018373504?utm_source=tag-newest
            // 它和mp3文件有关，是一个流的标志
            if (videoDecoder && (videoDecoder->getStream()->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
                AVPacket copy = {nullptr};
                if (av_packet_ref(&copy, &videoDecoder->getStream()->attached_pic) < 0) {
                    break;
                }
                videoDecoder->pushPacket(&copy);
            }
            attachmentRequest = 0;
        }

        if (isNoReadMore()) {
            waitCondition.waitRelative(waitMutex, 10);
            continue;
        }

        if (isRetryPlay()) {
            if (playerState->loop) {
                seekTo(playerState->startTime != AV_NOPTS_VALUE ? playerState->startTime : 0);
            } else if (playerState->autoExit) {
                ret = ERROR_EOF;
                break;
            }
        }

        // 读出数据包
        ret = av_read_frame(formatContext, pkt);

        if (ret < 0) {
            // 如果没能读出裸数据包，判断是否是结尾
            if ((ret == AVERROR_EOF || avio_feof(formatContext->pb)) && !eof) {
                if (videoDecoder) {
                    videoDecoder->pushNullPacket();
                }
                if (audioDecoder) {
                    audioDecoder->pushNullPacket();
                }
                eof = 1;
            }
            // 读取出错，则直接退出
            if (formatContext->pb && formatContext->pb->error) {
                ALOGE(TAG, "%s I/O context error ", __func__);
                ret = ERROR_IO;
                break;
            }
            waitCondition.waitRelative(waitMutex, 10);
            continue;
        } else {
            eof = 0;
        }

        if (audioDecoder &&
            pkt->stream_index == audioDecoder->getStreamIndex() &&
            isPacketInPlayRange(formatContext, pkt)) {
            audioDecoder->pushPacket(pkt);
        } else if (videoDecoder &&
                   pkt->stream_index == videoDecoder->getStreamIndex() &&
                   isPacketInPlayRange(formatContext, pkt)) {
            videoDecoder->pushPacket(pkt);
        } else {
            av_packet_unref(pkt);
        }
    } // end for

    if (audioDecoder) {
        audioDecoder->stop();
    }
    if (videoDecoder) {
        videoDecoder->stop();
    }
    if (audioDevice) {
        audioDevice->stop();
    }
    if (mediaSync) {
        mediaSync->stop();
    }
    quit = true;
    condition.signal();
    return ret;
}

bool MediaPlayer::isPacketInPlayRange(const AVFormatContext *formatContext, const AVPacket *packet) const {

    /* check if packet playerState in stream range specified by user, then packetQueue, otherwise discard */
    int64_t streamStartTime = formatContext->streams[packet->stream_index]->start_time;

    // https://baike.baidu.com/item/PTS/13977433
    int64_t packetTimestamp = (packet->pts == AV_NOPTS_VALUE) ? packet->dts : packet->pts;
    int64_t diffTimestamp = packetTimestamp - (streamStartTime != AV_NOPTS_VALUE ? streamStartTime : 0);

    double diffTime = diffTimestamp * av_q2d(formatContext->streams[packet->stream_index]->time_base);
    double startTime = (double) (playerState->startTime != AV_NOPTS_VALUE ? playerState->startTime : 0) / AV_TIME_BASE;
    double duration = (double) playerState->duration / AV_TIME_BASE;

    return (playerState->duration == AV_NOPTS_VALUE) || ((diffTime - startTime) <= duration);
}

bool MediaPlayer::isRetryPlay() const {
    bool isNoPaused = !playerState->pauseRequest;
    bool isNoUseAudioFrame = !audioDecoder || audioDecoder->isFinished();
    bool isNoUseVideoFrame = !videoDecoder || audioDecoder->isFinished();
    return isNoPaused && isNoUseAudioFrame && isNoUseVideoFrame;
}

bool MediaPlayer::isNoReadMore() const {
    bool isNoInfiniteBuffer = playerState->infiniteBuffer < 1;
    bool isNoEnoughMemory =
            (audioDecoder ? audioDecoder->getMemorySize() : 0) + (videoDecoder ? videoDecoder->getMemorySize() : 0) >
            MAX_QUEUE_SIZE;
    bool isAudioEnoughPackets = !audioDecoder || audioDecoder->hasEnoughPackets();
    bool isVideoEnoughPackets = !videoDecoder || videoDecoder->hasEnoughPackets();
    return isNoInfiniteBuffer && (isNoEnoughMemory || (isAudioEnoughPackets && isVideoEnoughPackets));
}

int MediaPlayer::prepareDecoder(int streamIndex) {

    AVCodecContext *codecContext;
    AVCodec *codec = nullptr;
    AVDictionary *opts = nullptr;
    AVDictionaryEntry *t = nullptr;
    int ret = 0;
    const char *forcedCodecName = nullptr;

    // 判断流索引的合法性
    if (streamIndex < 0 || streamIndex >= formatContext->nb_streams) {
        ALOGE(TAG, "%s illegal stream index", __func__);
        return ERROR_STREAM_INDEX;
    }

    // 创建解码上下文
    codecContext = avcodec_alloc_context3(nullptr);
    if (!codecContext) {
        ALOGE(TAG, "%s alloc codec context failure", __func__);
        return ERROR_NOT_MEMORY;
    }

    do {
        // 复制解码上下文参数
        ret = avcodec_parameters_to_context(codecContext, formatContext->streams[streamIndex]->codecpar);
        if (ret < 0) {
            ALOGE(TAG, "%s copy codec params to context failure", __func__);
            ret = ERROR_COPY_CODEC_PARAM_TO_CONTEXT;
            break;
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
            ALOGD("forceCodecName = %s", forcedCodecName);
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
            ret = ERROR_NOT_FOUND_DCODE;
            break;
        }
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
            ret = ERROR_NOT_OPEN_DECODE;
            break;
        }

        if ((t = av_dict_get(opts, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
            ALOGE(TAG, "%s option %s not found", __func__, t->key);
            ret = ERROR_CODEC_OPTIONS;
            break;
        }

        // 根据解码器类型创建解码器
        formatContext->streams[streamIndex]->discard = AVDISCARD_DEFAULT;

        if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioDecoder = new AudioDecoder(codecContext,
                                            formatContext->streams[streamIndex],
                                            streamIndex,
                                            playerState, &flushPacket, &waitCondition);
        } else if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoDecoder = new VideoDecoder(formatContext,
                                            codecContext,
                                            formatContext->streams[streamIndex],
                                            streamIndex,
                                            playerState, &flushPacket, &waitCondition);
            attachmentRequest = 1;
        }
    } while (false);

    // 准备失败，则需要释放创建的解码上下文
    if (ret < 0) {
        ALOGE(TAG, "%s prepare decoder failure index = %s", __func__, streamIndex);
        avcodec_free_context(&codecContext);
    }

    // 释放参数
    av_dict_free(&opts);

    return ret;
}

void audioPCMQueueCallback(void *opaque, uint8_t *stream, int len) {
    MediaPlayer *mediaPlayer = (MediaPlayer *) opaque;
    mediaPlayer->pcmQueueCallback(stream, len);
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

void MediaPlayer::pcmQueueCallback(uint8_t *stream, int len) {
    if (!audioResampler) {
        memset(stream, 0, sizeof(len));
        return;
    }
    audioResampler->pcmQueueCallback(stream, len);
    if (playerState->msgQueue && playerState->syncType != AV_SYNC_VIDEO) {
        playerState->msgQueue->notifyMsg(MSG_CURRENT_POSITON, getCurrentPosition(), playerState->videoDuration);
    }
}

void MediaPlayer::setAudioDevice(AudioDevice *audioDevice) {
    Mutex::Autolock lock(mutex);
    MediaPlayer::audioDevice = audioDevice;
}

void MediaPlayer::setMediaSync(MediaSync *mediaSync) {
    this->mediaSync = mediaSync;
    if (this->mediaSync) {
        this->mediaSync->setPlayerState(playerState);
    }
}

MediaSync *MediaPlayer::getMediaSync() const {
    return mediaSync;
}

void MediaPlayer::setVideoDevice(VideoDevice *videoDevice) {
    Mutex::Autolock lock(mutex);
    MediaPlayer::videoDevice = videoDevice;
    mediaSync->setVideoDevice(videoDevice);
}
