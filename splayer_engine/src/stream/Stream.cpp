#include <stream/Stream.h>

Stream::Stream(MediaPlayer *mediaPlayer, PlayerState *playerState) {
    this->mediaPlayer = mediaPlayer;
    this->playerState = playerState;

    avformat_network_init();
    av_init_packet(&flushPacket);
    flushPacket.data = (uint8_t *) &flushPacket;
}

Stream::~Stream() {
    flushPacket.data = nullptr;
    av_packet_unref(&flushPacket);
    avformat_network_deinit();
}

int Stream::start() {
    ALOGD(TAG, __func__);
    if (!readThread) {
        readThread = new Thread(this);
        if (readThread) {
            readThread->start();
            return SUCCESS;
        }
        return ERROR_NOT_MEMORY;
    }
    return SUCCESS;
}

int Stream::stop() {
    ALOGD(TAG, __func__);
    if (readThread != nullptr) {
        readThread->join();
        delete readThread;
        readThread = nullptr;
    }
    return SUCCESS;
}

void Stream::run() {
    if (mediaPlayer) {
        if (openStream() < 0) {
            ALOGE(TAG, "%s open stream failure", __func__);
            return;
        }
        notifyMsg(Msg::MSG_PREPARED);
        ALOGD(TAG, "%s start prepare read packets", __func__);
        readPackets();
    } else {
        ALOGE(TAG, "%s media player is null", __func__);
    }
}

int Stream::readPackets() {
    ALOGD(TAG, __func__);

    // 读数据包流程
    int ret = 0;

    AVPacket pkt1, *pkt = &pkt1;

    bool isNoReadMoreLog = false;

    for (;;) {

        // 退出播放器
        if (playerState->abortRequest) {
            ALOGD(TAG, "%s exit read packet", __func__);
            break;
        }

        // 是否暂停
        if (playerState->pauseRequest != playerState->lastPaused) {
            playerState->lastPaused = playerState->pauseRequest;
            if (playerState->pauseRequest) {
                playerState->readPauseReturn = av_read_pause(formatContext);
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

        // 队列满，等待消耗
        if (isNoReadMore()) {
            waitCondition.waitRelative(waitMutex, 10);
            if (!isNoReadMoreLog) {
                isNoReadMoreLog = true;
                ALOGD(TAG, "%s not need read more, wait 10", __func__);
            }
            continue;
        }
        isNoReadMoreLog = false;

        if (isRetryPlay()) {
            if (playerState->loop) {
//                seekTo(playerState->startTime != AV_NOPTS_VALUE ? playerState->startTime : 0);
            } else if (playerState->autoExit) {
                ret = ERROR_EOF;
                ALOGD(TAG, "%s exit eof", __func__);
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

//    if (audioDecoder) {
//        audioDecoder->stop();
//    }
//    if (videoDecoder) {
//        videoDecoder->stop();
//    }
//    if (audioDevice) {
//        audioDevice->stop();
//    }
//    if (mediaSync) {
//        mediaSync->stop();
//    }
//    quit = true;
    condition.signal();
    return ret;
}

static int avFormatInterruptCb(void *ctx) {
    PlayerState *playerState = (PlayerState *) ctx;
    if (playerState->abortRequest) {
        return AVERROR_EOF;
    }
    return 0;
}

int Stream::openStream() {

    if (mediaPlayer) {
        mediaPlayer->onStartOpenStream();
    }

    AVDictionaryEntry *t;
    AVDictionary **opts;
    int scanAllProgramMapTableSet = 0;
    int ret = 0;

    // 创建解复用上下文
    formatContext = avformat_alloc_context();
    if (!formatContext) {
        ALOGE(TAG, "%s avformat could not allocate context", __func__);
        closeStream();
        notifyMsg(Msg::MSG_ERROR, ERROR_NOT_MEMORY);
        return ERROR_NOT_MEMORY;
    }
    mediaPlayer->setFormatContext(formatContext);

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
    if (av_stristart(playerState->url, "rtmp", nullptr) ||
        av_stristart(playerState->url, "rtsp", nullptr)) {
        // There is total different meaning for 'timeout' option in rtmp
        ALOGD(TAG, "%s remove 'timeout' option for rtmp.", __func__);
        av_dict_set(&playerState->format_opts, "timeout", nullptr, 0);
    }

    // 打开文件
    ret = avformat_open_input(&formatContext,
                              playerState->url,
                              playerState->inputFormat,
                              &playerState->format_opts);
    if (ret < 0) {
        ALOGE(TAG, "%s avformat could not open input", __func__);
        closeStream();
        notifyMsg(Msg::MSG_ERROR, ERROR_NOT_OPEN_INPUT);
        return ERROR_NOT_OPEN_INPUT;
    }

    // 还原MPEGTS的特殊处理标记
    if (scanAllProgramMapTableSet) {
        av_dict_set(&playerState->format_opts, OPT_SCALL_ALL_PMTS, nullptr, AV_DICT_MATCH_CASE);
    }

    if ((t = av_dict_get(playerState->format_opts, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
        ALOGE(TAG, "%s Option %s not found", __func__, t->key);
        closeStream();
        notifyMsg(Msg::MSG_ERROR, ERROR_CODEC_OPTIONS);
        return ERROR_CODEC_OPTIONS;
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
        ALOGE(TAG, "%s %s: could not find codec parameters", __func__, playerState->url);
        closeStream();
        notifyMsg(Msg::MSG_ERROR, ERROR_NOT_FOUND_STREAM_INFO);
        return ERROR_NOT_FOUND_STREAM_INFO;
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

    // get window title from metadata
    if (!playerState->videoTitle && (t = av_dict_get(formatContext->metadata, "title", nullptr, 0))) {
        playerState->videoTitle = av_asprintf("%s - %s", t->value, playerState->url);
    }

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

    if (mediaPlayer) {
        mediaPlayer->onEndOpenStream(videoIndex, audioIndex);
    }

    // 如果音频流和视频流都没有找到，则直接退出
    if (audioIndex == -1 && videoIndex == -1) {
        ALOGE(TAG, "%s could not find audio and video stream", __func__);

        closeStream();
        notifyMsg(Msg::MSG_ERROR, ERROR_DISABLE_ALL_STREAM);
        return ERROR_DISABLE_ALL_STREAM;
    }

    return SUCCESS;
}


bool Stream::isPacketInPlayRange(const AVFormatContext *formatContext, const AVPacket *packet) const {

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

bool Stream::isRetryPlay() const {
    bool isNoPaused = !playerState->pauseRequest;
    bool isNoUseAudioFrame = !audioDecoder || audioDecoder->isFinished();
    bool isNoUseVideoFrame = !videoDecoder || audioDecoder->isFinished();
    return isNoPaused && isNoUseAudioFrame && isNoUseVideoFrame;
}

bool Stream::isNoReadMore() const {
    bool isNoInfiniteBuffer = playerState->infiniteBuffer < 1;
    bool isNoEnoughMemory =
            (audioDecoder ? audioDecoder->getMemorySize() : 0) + (videoDecoder ? videoDecoder->getMemorySize() : 0) >
            MAX_QUEUE_SIZE;
    bool isAudioEnoughPackets = !audioDecoder || audioDecoder->hasEnoughPackets();
    bool isVideoEnoughPackets = !videoDecoder || videoDecoder->hasEnoughPackets();
    return isNoInfiniteBuffer && (isNoEnoughMemory || (isAudioEnoughPackets && isVideoEnoughPackets));
}

void Stream::setAttachmentRequest(int attachmentRequest) {
    Stream::attachmentRequest = attachmentRequest;
}

void Stream::setAudioDecoder(AudioDecoder *audioDecoder) {
    Stream::audioDecoder = audioDecoder;
}

void Stream::setVideoDecoder(VideoDecoder *videoDecoder) {
    Stream::videoDecoder = videoDecoder;
}

void Stream::setMediaSync(MediaSync *mediaSync) {
    Stream::mediaSync = mediaSync;
}

Condition *Stream::getWaitCondition() {
    return &waitCondition;
}

Mutex *Stream::getWaitMutex() {
    return &waitMutex;
}

AVPacket *Stream::getFlushPacket() {
    return &flushPacket;
}

void Stream::setFormatContext(AVFormatContext *formatContext) {
    Stream::formatContext = formatContext;
}

int Stream::notifyMsg(int what) {
    if (playerState) {
        playerState->notifyMsg(what);
        return SUCCESS;
    }
    return ERROR;
}

int Stream::notifyMsg(int what, int arg1) {
    if (playerState) {
        playerState->notifyMsg(what, arg1);
        return SUCCESS;
    }
    return ERROR;
}

int Stream::notifyMsg(int what, int arg1, int arg2) {
    if (playerState) {
        playerState->notifyMsg(what, arg1, arg2);
        return SUCCESS;
    }
    return ERROR;
}

void Stream::closeStream() {

}

void Stream::setStreamListener(IStreamListener *streamListener) {
    Stream::streamListener = streamListener;
}

void Stream::setEof(int eof) {
    Stream::eof = eof;
}
