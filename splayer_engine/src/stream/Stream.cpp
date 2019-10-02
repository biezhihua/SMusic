#include <stream/Stream.h>

static int avFormatInterruptCb(void *ctx) {
    if (ctx) {
        auto *playerState = (PlayerState *) ctx;
        if (playerState->abortRequest) {
            return AVERROR_EOF;
        }
    }
    // not modify
    return 0;
}

Stream::Stream(MediaPlayer *mediaPlayer, PlayerState *playerState) {
    this->mediaPlayer = mediaPlayer;
    this->playerState = playerState;
}

Stream::~Stream() {
    mediaPlayer = nullptr;
    playerState = nullptr;
    streamListener = nullptr;
    mediaSync = nullptr;
}

int Stream::start() {
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
    audioDecoder = nullptr;
    videoDecoder = nullptr;
    if (formatContext) {
        avformat_close_input(&formatContext);
        avformat_free_context(formatContext);
        formatContext = nullptr;
    }
    if (readThread) {
        readThread->join();
        delete readThread;
        readThread = nullptr;
    }
    return SUCCESS;
}

void Stream::run() {
    if (mediaPlayer) {
        if (openStream() < 0) {
            if (DEBUG) {
                ALOGE(TAG, "%s open stream failure", __func__);
            }
            return;
        }
        notifyMsg(Msg::MSG_STARTED);
        if (readPackets() < 0) {
            if (DEBUG) {
                ALOGD(TAG, "%s read packets exit", __func__);
            }
        }
    }
}

int Stream::readPackets() {
    AVPacket pkt1, *pkt = &pkt1;

    for (;;) {

        if (!playerState || !formatContext) {
            if (DEBUG) {
                ALOGD(TAG, "%s error state", __func__);
            }
            return ERROR;
        }

        // 退出播放器
        if (playerState->abortRequest) {
            if (DEBUG) {
                ALOGD(TAG, "%s abort request, exit read packet", __func__);
            }
            return ERROR_ABORT_REQUEST;
        }

        // 是否暂停
        if (playerState->pauseRequest != playerState->lastPaused) {
            doPause();
        }

        // 处理定位请求
        if (playerState->seekRequest) {
            doSeek();
        }

        // 处理封面数据包
        if (playerState->attachmentRequest) {
            if (doAttachment() < 0) {
                if (DEBUG) {
                    ALOGE(TAG, "%s do attachment fail, exit read thread", __func__);
                }
                notifyMsg(Msg::MSG_ERROR, ERROR_ATTACHMENT);
                notifyMsg(Msg::MSG_REQUEST_STOP);
                return ERROR_ATTACHMENT;
            }
        }

        // 队列满，等待消耗
        if (isNotReadMore()) {
            waitMutex.lock();
            waitCondition.waitRelative(waitMutex, 10);
            waitMutex.unlock();
            continue;
        }

        if (isRetryPlay()) {
            doRetryPlay();
        }

        // 读出数据包
        int ret = av_read_frame(formatContext, pkt);

        if (ret < 0) {

            // 如果没能读出裸数据包，判断是否是结尾
            if ((ret == AVERROR_EOF || avio_feof(formatContext->pb)) && !playerState->eof) {
                if (videoDecoder) {
                    videoDecoder->pushNullPacket();
                }
                if (audioDecoder) {
                    audioDecoder->pushNullPacket();
                }
                playerState->eof = 1;
            }

            // 读取出错，则直接退出
            if (formatContext->pb && formatContext->pb->error) {
                if (DEBUG) {
                    ALOGE(TAG, "%s I/O context error ", __func__);
                }
                notifyMsg(Msg::MSG_ERROR, ERROR_IO);
                notifyMsg(Msg::MSG_REQUEST_STOP);
                return ERROR_IO;
            }

            waitMutex.lock();
            waitCondition.waitRelative(waitMutex, 10);
            waitMutex.unlock();

            continue;
        } else {
            playerState->eof = 0;
        }

        if (audioDecoder && pkt->stream_index == audioDecoder->getStreamIndex() && isPacketInPlayRange(formatContext, pkt)) {
            audioDecoder->pushPacket(pkt);
        } else if (videoDecoder && pkt->stream_index == videoDecoder->getStreamIndex() && isPacketInPlayRange(formatContext, pkt)) {
            videoDecoder->pushPacket(pkt);
        } else {
            av_packet_unref(pkt);
        }
    }
}

void Stream::doRetryPlay() {
    if (playerState) {
        if (playerState->loopTimes) {
            notifyMsg(Msg::MSG_REQUEST_SEEK, (int) (playerState->startTime != AV_NOPTS_VALUE ? playerState->startTime : 0));
        } else if (playerState->autoExit) {
            notifyMsg(Msg::MSG_REQUEST_DESTROY);
        }
    }
}

int Stream::doAttachment() const {
    if (playerState && videoDecoder) {
        // https://segmentfault.com/a/1190000018373504?utm_source=tag-newest
        // 它和mp3文件有关，是一个流的标志
        if (videoDecoder && (videoDecoder->getStream()->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
            AVPacket copy = {nullptr};
            if (av_packet_ref(&copy, &videoDecoder->getStream()->attached_pic) < 0) {
                return ERROR_ATTACHMENT;
            }
            videoDecoder->pushPacket(&copy);
        }
        playerState->attachmentRequest = 0;
    }
    return SUCCESS;
}

void Stream::doPause() const {
    if (playerState && formatContext) {
        playerState->lastPaused = playerState->pauseRequest;
        if (playerState->pauseRequest) {
            playerState->readPauseReturn = av_read_pause(formatContext);
        } else {
            av_read_play(formatContext);
        }
    }
}

void Stream::doSeek() const {
    if (playerState && formatContext) {
        int64_t seekTarget = playerState->seekPos;

        int64_t seekMin = playerState->seekRel > 0 ? seekTarget - playerState->seekRel + 2 : INT64_MIN;

        int64_t seekMax = playerState->seekRel < 0 ? seekTarget - playerState->seekRel - 2 : INT64_MAX;

        // 定位
        playerState->mutex.lock();
        int ret = avformat_seek_file(formatContext, -1, seekMin, seekTarget, seekMax, playerState->seekFlags);
        playerState->mutex.unlock();

        if (ret < 0) {
            if (DEBUG) {
                ALOGD(TAG, "%s %s: error while seeking", __func__, playerState->url);
            }
        } else {
            if (audioDecoder) {
                audioDecoder->flush();
                audioDecoder->pushFlushPacket();
                if (DEBUG) {
                    ALOGD(TAG, "%s flush audio", __func__);
                }
            }
            if (videoDecoder) {
                videoDecoder->flush();
                videoDecoder->pushFlushPacket();
                if (DEBUG) {
                    ALOGD(TAG, "%s flush video", __func__);
                }
            }
            // 更新外部时钟值
            if (playerState->seekFlags & AVSEEK_FLAG_BYTE) {
                if (mediaSync) {
                    mediaSync->updateExternalClock(NAN, 0);
                }
            } else {
                if (mediaSync) {
                    mediaSync->updateExternalClock(seekTarget / (double) AV_TIME_BASE, 0);
                }
            }
        }
        playerState->attachmentRequest = 1;
        playerState->seekRequest = 0;
        playerState->eof = 0;
    }
}

int Stream::openStream() {

    AVDictionaryEntry *t;
    AVDictionary **opts;
    int scanAllProgramMapTableSet = 0;
    int ret = 0;

    if (streamListener) {
        streamListener->onStartOpenStream();
    }

    // 创建解复用上下文
    formatContext = avformat_alloc_context();
    if (!formatContext) {
        if (DEBUG) {
            ALOGE(TAG, "%s avformat could not allocate context", __func__);
        }
        notifyMsg(Msg::MSG_ERROR, ERROR_NOT_MEMORY);
        notifyMsg(Msg::MSG_REQUEST_STOP);
        return ERROR_NOT_MEMORY;
    }

    // 设置上下文
    playerState->setFormatContext(formatContext);
    mediaPlayer->setFormatContext(formatContext);

    // 设置解复用中断回调
    formatContext->interrupt_callback.callback = avFormatInterruptCb;
    formatContext->interrupt_callback.opaque = playerState;

    // https://zhuanlan.zhihu.com/p/43672062
    // scan_all_pmts, 是mpegts的一个选项，这里在没有设定该选项的时候，强制设为1
    // scan_all_pmts, 扫描全部的ts流的"Program Map Table"表。
    if (!av_dict_get(playerState->formatOpts, OPT_SCALL_ALL_PMTS, nullptr, AV_DICT_MATCH_CASE)) {
        av_dict_set(&playerState->formatOpts, OPT_SCALL_ALL_PMTS, "1", AV_DICT_DONT_OVERWRITE);
        scanAllProgramMapTableSet = 1;
    }

    // 处理文件头
    if (playerState->headers) {
        av_dict_set(&playerState->formatOpts, OPT_HEADERS, playerState->headers, 0);
    }

    // 处理文件偏移量
    if (playerState->offset > 0) {
        formatContext->skip_initial_bytes = playerState->offset;
    }

    // 设置rtmp/rtsp的超时值
    if (av_stristart(playerState->url, FORMAT_RTMP, nullptr) || av_stristart(playerState->url, FORMAT_RTSP, nullptr)) {
        // There is total different meaning for 'timeout' option in rtmp
        if (DEBUG) {
            ALOGD(TAG, "%s remove 'timeout' option for rtmp.", __func__);
        }
        av_dict_set(&playerState->formatOpts, OPT_KEY_TIMEOUT, nullptr, 0);
    }

    // 打开文件
    ret = avformat_open_input(&formatContext, playerState->url, playerState->inputFormat, &playerState->formatOpts);
    if (ret < 0) {
        if (DEBUG) {
            ALOGE(TAG, "%s avformat could not open input", __func__);
        }
        notifyMsg(Msg::MSG_ERROR, ERROR_NOT_OPEN_INPUT);
        notifyMsg(Msg::MSG_REQUEST_STOP);
        return ERROR_NOT_OPEN_INPUT;
    }

    // 还原MPEGTS的特殊处理标记
    if (scanAllProgramMapTableSet) {
        av_dict_set(&playerState->formatOpts, OPT_SCALL_ALL_PMTS, nullptr, AV_DICT_MATCH_CASE);
    }

    if ((t = av_dict_get(playerState->formatOpts, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
        if (DEBUG) ALOGE(TAG, "%s Option %s not found", __func__, t->key);
        notifyMsg(Msg::MSG_ERROR, ERROR_CODEC_OPTIONS);
        notifyMsg(Msg::MSG_REQUEST_STOP);
        return ERROR_CODEC_OPTIONS;
    }

    if (playerState->generateMissingPts) {
        formatContext->flags |= AVFMT_FLAG_GENPTS;
    }
    av_format_inject_global_side_data(formatContext);

    opts = setupStreamInfoOptions(formatContext, playerState->codecOpts);

    // 查找媒体流信息
    ret = avformat_find_stream_info(formatContext, opts);
    if (opts) {
        for (int i = 0; i < formatContext->nb_streams; i++) {
            if (opts[i]) {
                av_dict_free(&opts[i]);
            }
        }
        av_freep(&opts);
    }

    if (ret < 0) {
        if (DEBUG) {
            ALOGE(TAG, "%s %s: could not find codec parameters", __func__, playerState->url);
        }
        notifyMsg(Msg::MSG_ERROR, ERROR_NOT_FOUND_STREAM_INFO);
        notifyMsg(Msg::MSG_REQUEST_STOP);
        return ERROR_NOT_FOUND_STREAM_INFO;
    }

    // 判断是否实时流，判断是否需要设置无限缓冲区
    playerState->realTime = isRealTime(formatContext);
    if (playerState->infiniteBuffer < 0 && playerState->realTime) {
        playerState->infiniteBuffer = 1;
    }

    // Gets the duration of the file, -1 if no duration available.
    if (playerState->realTime) {
        playerState->duration = -1;
    } else {
        playerState->duration = -1;
        if (formatContext->duration) {
            playerState->duration = formatContext->duration;
            playerState->durationSec = av_rescale(formatContext->duration, 1000, AV_TIME_BASE);
        }
    }

    // I/O context.
    if (formatContext->pb) {
        formatContext->pb->eof_reached = 0;
    }

    // 判断是否以字节方式定位
    playerState->seekByBytes = (formatContext->iformat->flags & AVFMT_TS_DISCONT) != 0 && strcmp(FORMAT_OGG, formatContext->iformat->name) != 0;

    // get window title from metadata
    if (!playerState->videoTitle && (t = av_dict_get(formatContext->metadata, "title", nullptr, 0))) {
        playerState->videoTitle = av_asprintf("%s - %s", t->value, playerState->url);
    }

    // 设置最大帧间隔
    mediaSync->setMaxDuration((formatContext->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0);

    // 如果不是从头开始播放，则跳转到播放位置
    if (playerState->startTime != AV_NOPTS_VALUE) {
        int64_t timestamp = playerState->startTime;
        if (formatContext->start_time != AV_NOPTS_VALUE) {
            timestamp += formatContext->start_time;
        }
        playerState->mutex.lock();
        ret = avformat_seek_file(formatContext, -1, INT64_MIN, timestamp, INT64_MAX, 0);
        playerState->mutex.unlock();
        if (ret < 0) {
            if (DEBUG) {
                ALOGE(TAG, "%s %s: could not seek to position %0.3f", __func__, playerState->url, (double) timestamp / AV_TIME_BASE);
            }
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

    if (streamListener) {
        streamListener->onEndOpenStream(videoIndex, audioIndex);
    }

    // 如果音频流和视频流都没有找到，则直接退出
    if (audioIndex == -1 && videoIndex == -1) {
        if (DEBUG) {
            ALOGE(TAG, "%s could not find audio and video stream", __func__);
        }
        notifyMsg(Msg::MSG_ERROR, ERROR_DISABLE_ALL_STREAM);
        notifyMsg(Msg::MSG_REQUEST_STOP);
        return ERROR_DISABLE_ALL_STREAM;
    }

    return SUCCESS;
}

bool Stream::isPacketInPlayRange(const AVFormatContext *formatContext, const AVPacket *packet) const {
    if (playerState) {

        if (playerState->duration == AV_NOPTS_VALUE) {
            return SUCCESS;
        }

        /* check if packet playerState in stream range specified by user, then
         * packetQueue, otherwise discard */
        int64_t streamStartTime = formatContext->streams[packet->stream_index]->start_time;

        // https://baike.baidu.com/item/PTS/13977433
        int64_t packetTimestamp = (packet->pts == AV_NOPTS_VALUE) ? packet->dts : packet->pts;
        int64_t diffTimestamp = packetTimestamp - (streamStartTime != AV_NOPTS_VALUE ? streamStartTime : 0);

        double diffTime = diffTimestamp * av_q2d(formatContext->streams[packet->stream_index]->time_base);
        double startTime = (double) (playerState->startTime != AV_NOPTS_VALUE ? playerState->startTime : 0) / AV_TIME_BASE;
        double duration = (double) playerState->duration / AV_TIME_BASE;

        return (playerState->duration == AV_NOPTS_VALUE) || ((diffTime - startTime) <= duration);
    }
    return false;
}

bool Stream::isRetryPlay() const {
    bool isNoPaused = !playerState->pauseRequest;
    bool isNoUseAudioFrame = !audioDecoder || audioDecoder->isFinished();
    bool isNoUseVideoFrame = !videoDecoder || videoDecoder->isFinished();
    return isNoPaused && isNoUseAudioFrame && isNoUseVideoFrame;
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

Condition *Stream::getWaitCondition() { return &waitCondition; }

AVPacket *Stream::getFlushPacket() { return &flushPacket; }

int Stream::notifyMsg(int what) {
    if (messageCenter) {
        return messageCenter->notifyMsg(what);
    }
    return ERROR;
}

int Stream::notifyMsg(int what, int arg1) {
    if (messageCenter) {
        return messageCenter->notifyMsg(what, arg1);
    }
    return ERROR;
}

int Stream::notifyMsg(int what, int arg1, int arg2) {
    if (messageCenter) {
        return messageCenter->notifyMsg(what, arg1, arg2);
    }
    return ERROR;
}

void Stream::setStreamListener(IStreamListener *streamListener) {
    Stream::streamListener = streamListener;
}

int Stream::create() {
    avformat_network_init();
    av_init_packet(&flushPacket);
    flushPacket.data = (uint8_t *) &flushPacket;
    return SUCCESS;
}

int Stream::destroy() {
    flushPacket.data = nullptr;
    av_packet_unref(&flushPacket);
    avformat_network_deinit();
    streamListener = nullptr;
    mediaSync = nullptr;
    return SUCCESS;
}

void Stream::setMessageCenter(MessageCenter *messageCenter) {
    Stream::messageCenter = messageCenter;
}

bool Stream::isNotReadMore() const {
    bool isNoInfiniteBuffer = playerState->infiniteBuffer < 1;
    bool isNoEnoughMemory = (audioDecoder ? audioDecoder->getPacketQueueMemorySize() : 0) + (videoDecoder ? videoDecoder->getPacketQueueMemorySize() : 0) > MAX_QUEUE_SIZE;
    bool isAudioEnoughPackets = !audioDecoder || audioDecoder->hasEnoughPackets();
    bool isVideoEnoughPackets = !videoDecoder || videoDecoder->hasEnoughPackets();
    bool isNotReadMore = isNoInfiniteBuffer && (isNoEnoughMemory || (isAudioEnoughPackets && isVideoEnoughPackets));
    return isNotReadMore;
}
