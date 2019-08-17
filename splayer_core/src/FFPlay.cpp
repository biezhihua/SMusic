#include "FFPlay.h"

FFPlay::FFPlay() {
    msgQueue = new MessageQueue();
}

FFPlay::~FFPlay() {
    delete stream;
    delete audio;
    delete msgQueue;
}

void FFPlay::setAudio(Audio *audio) {
    ALOGD(FFPLAY_TAG, __func__);
    FFPlay::audio = audio;
}

void FFPlay::setSurface(Surface *surface) {
    ALOGD(FFPLAY_TAG, __func__);
    FFPlay::surface = surface;
}

void FFPlay::setStream(Stream *stream) {
    ALOGD(FFPLAY_TAG, __func__);
    FFPlay::stream = stream;
}

MessageQueue *FFPlay::getMsgQueue() const {
    return msgQueue;
}

int FFPlay::stop() {
    // TODO
    return NEGATIVE_UNKNOWN;
}

int FFPlay::shutdown() {
    // TODO
    waitStop();
    return NEGATIVE_UNKNOWN;
}

int FFPlay::waitStop() {
    // TODO
    return NEGATIVE_UNKNOWN;
}

int FFPlay::prepareStream(const char *fileName) {
    ALOGD(FFPLAY_TAG, __func__);

    showVersionsAndOptions();

    av_init_packet(&flushPacket);
    flushPacket.data = (uint8_t *) &flushPacket;

    options->inputFileName = fileName != nullptr ? av_strdup(fileName) : nullptr;

    videoState = streamOpen();

    if (!videoState) {
        return NEGATIVE(S_NOT_CREATE_VIDEO_STATE);
    }

    return POSITIVE;
}

void FFPlay::showVersionsAndOptions() {
    ALOGD(FFPLAY_TAG, "===== versions =====");
    ALOGD(FFPLAY_TAG, "%-*s: %s", VERSION_MODULE_FILE_NAME_LENGTH, "FFmpeg", av_version_info());
    ALOGD(FFPLAY_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "libavutil", avutil_version());
    ALOGD(FFPLAY_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "libavcodec", avcodec_version());
    ALOGD(FFPLAY_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "libavformat", avformat_version());
    ALOGD(FFPLAY_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "libswscale", swscale_version());
    ALOGD(FFPLAY_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "libswresample", swresample_version());

    ALOGD(FFPLAY_TAG, "===== options =====");
    showDict("player-opts", options->playerDict);
    showDict("format-opts", options->format);
    showDict("codec-opts ", options->codec);
    showDict("sws-opts   ", options->swsDict);
    showDict("swr-opts   ", options->swrDict);
}

int FFPlay::getMsg(Message *msg, bool block) {
    while (true) {
        bool continueWaitNextMsg = false;
        int ret = msgQueue->getMsg(msg, block);
        if (ret != POSITIVE) {
            return ret;
        }
        switch (msg->what) {
            case Message::MSG_PREPARED:
                break;
            case Message::MSG_COMPLETED:
                break;
            case Message::MSG_SEEK_COMPLETE:
                break;
            case Message::REQ_START:
                break;
            case Message::REQ_PAUSE:
                break;
            case Message::REQ_SEEK:
                break;
            default:
                break;
        }
        if (continueWaitNextMsg) {
            msg->free();
            continue;
        }
        return ret;
    }
}

void FFPlay::showDict(const char *tag, AVDictionary *dict) {
    AVDictionaryEntry *t = nullptr;
    while ((t = av_dict_get(dict, "", t, AV_DICT_IGNORE_SUFFIX))) {
        ALOGD(FFPLAY_TAG, "%-*s: %-*s = %s\n", 12, tag, 28, t->key, t->value);
    }
}

static int innerReadThread(void *arg) {
    auto *play = static_cast<FFPlay *>(arg);
    if (play) {
        return play->readThread();
    }
    return POSITIVE;
}

VideoState *FFPlay::streamOpen() {
    ALOGD(FFPLAY_TAG, __func__);

    if (!options->inputFileName) {
        ALOGE(FFPLAY_TAG, "%s input file name is null", __func__);
        return nullptr;
    }

    auto *is = new VideoState();
    if (!is) {
        ALOGD(FFPLAY_TAG, "%s create video state oom", __func__);
        return nullptr;
    }

    if (is->videoFrameQueue.init(&is->videoPacketQueue, VIDEO_QUEUE_SIZE, 1) < 0) {
        ALOGE(FFPLAY_TAG, "%s video frame packetQueue init fail", __func__);
        streamClose();
        return nullptr;
    }

    if (is->audioFrameQueue.init(&is->audioPacketQueue, AUDIO_QUEUE_SIZE, 0) < 0) {
        ALOGE(FFPLAY_TAG, "%s audio frame packetQueue init fail", __func__);
        streamClose();
        return nullptr;
    }

    if (is->subtitleFrameQueue.init(&is->subtitlePacketQueue, SUBTITLE_QUEUE_SIZE, 0) < 0) {
        ALOGE(FFPLAY_TAG, "%s subtitle frame packetQueue init fail", __func__);
        streamClose();
        return nullptr;
    }

    if (is->videoPacketQueue.init(&flushPacket) < 0 ||
        is->audioPacketQueue.init(&flushPacket) < 0 ||
        is->subtitlePacketQueue.init(&flushPacket) < 0) {
        ALOGE(FFPLAY_TAG, "%s packet packetQueue init fail", __func__);
        streamClose();
        return nullptr;
    }

    if (!(is->continueReadThread = new Mutex())) {
        ALOGE(FFPLAY_TAG, "%s create continue read thread mutex fail", __func__);
        streamClose();
        return nullptr;
    }

    if (is->videoClock.init(&is->videoPacketQueue.serial) < 0 ||
            is->audioClock.init(&is->audioPacketQueue.serial) < 0 ||
            is->exitClock.init(&is->subtitlePacketQueue.serial) < 0) {
        ALOGE(FFPLAY_TAG, "%s init clock fail", __func__);
        streamClose();
        return nullptr;
    }

    is->fileName = av_strdup(options->inputFileName);
    is->audioClockSerial = -1;
    is->inputFormat = options->inputFormat;
    is->yTop = 0;
    is->xLeft = 0;
    is->audioVolume = getStartupVolume();
    is->muted = 0;
    is->syncType = options->syncType;
    is->readThread = new Thread(innerReadThread, this, "Read   ");

    if (!is->readThread) {
        ALOGE(FFPLAY_TAG, "%s create read thread fail", __func__);
        streamClose();
        return nullptr;
    }

    return is;
}

int FFPlay::getStartupVolume() {
    if (options->startupVolume < 0) {
        ALOGD(FFPLAY_TAG, "%s -volume=%d < 0, setting to 0", __func__, options->startupVolume);
    }
    if (options->startupVolume > 100) {
        ALOGD(FFPLAY_TAG, "%s -volume=%d > 100 0, setting to 100", __func__, options->startupVolume);
    }
    options->startupVolume = av_clip(options->startupVolume, 0, 100);
    options->startupVolume = av_clip(MIX_MAX_VOLUME * options->startupVolume / 100, 0, MIX_MAX_VOLUME);
    return options->startupVolume;
}

void FFPlay::streamClose() {
    ALOGD(FFPLAY_TAG, __func__);

    if (videoState) {

        /* XXX: use a special url_shutdown call to abort parse cleanly */

        videoState->abortRequest = 1;

        if (videoState->readThread) {
            videoState->readThread->waitThread();
        }

        // close all streams
        if (videoState->audioStreamIndex >= 0) {
            streamComponentClose(videoState->audioStream, videoState->audioStreamIndex);
        }
        if (videoState->videoStreamIndex >= 0) {
            streamComponentClose(videoState->videoStream, videoState->videoStreamIndex);
        }
        if (videoState->subtitleStreamIndex >= 0) {
            streamComponentClose(videoState->subtitleStream, videoState->subtitleStreamIndex);
        }

        // close stream input
        avformat_close_input(&videoState->formatContext);

        // free all packet
        videoState->videoPacketQueue.destroy();
        videoState->audioPacketQueue.destroy();
        videoState->subtitlePacketQueue.destroy();

        // free all frame
        videoState->videoFrameQueue.destroy();
        videoState->audioFrameQueue.destroy();
        videoState->subtitleFrameQueue.destroy();

        if (videoState->continueReadThread) {
            delete videoState->continueReadThread;
            videoState->continueReadThread = nullptr;
        }

        sws_freeContext(videoState->imgConvertCtx);
        sws_freeContext(videoState->subConvertCtx);

        av_free(videoState->fileName);

        // TODO Texture

        delete videoState;
        videoState = nullptr;
    }
}


static int decodeInterruptCallback(void *ctx) {
    auto *is = static_cast<VideoState *>(ctx);
    return is->abortRequest;
}

/* this thread gets the stream from the disk or the network */
int FFPlay::readThread() {
    ALOGD(FFPLAY_TAG, __func__);

    if (!videoState) {
        return NEGATIVE(S_NULL);
    }

    AVFormatContext *formatContext = nullptr;
    AVPacket packet1, *packet = &packet1;
    auto *waitMutex = new Mutex();
    AVDictionaryEntry *dictionaryEntry;
    int streamIndex[AVMEDIA_TYPE_NB] = {-1};
    int packetInPlayRange = 0;
    int scanAllPmtsSet = 0;
    int ret = 0;
    int64_t streamStartTime;
    int64_t packetTimestamp;

    if (!waitMutex) {
        ALOGE(FFPLAY_TAG, "%s create wait mutex fail", __func__);
        return NEGATIVE(S_NOT_MEMORY);
    }

    videoState->lastVideoStreamIndex = videoState->videoStreamIndex = -1;
    videoState->lastAudioStreamIndex = videoState->audioStreamIndex = -1;
    videoState->lastSubtitleStreamIndex = videoState->subtitleStreamIndex = -1;
    videoState->eof = 0;

    formatContext = avformat_alloc_context();
    if (!formatContext) {
        ALOGE(FFPLAY_TAG, "%s avformat could not allocate context", __func__);
        return NEGATIVE(S_NOT_MEMORY);
    }

    formatContext->interrupt_callback.callback = decodeInterruptCallback;
    formatContext->interrupt_callback.opaque = videoState;

    if (!av_dict_get(options->format, SCAN_ALL_PMTS, nullptr, AV_DICT_MATCH_CASE)) {
        av_dict_set(&options->format, SCAN_ALL_PMTS, "1", AV_DICT_DONT_OVERWRITE);
        scanAllPmtsSet = 1;
    }

    if (options->inputFormatName) {
        videoState->inputFormat = av_find_input_format(options->inputFormatName);
    }

    if (avformat_open_input(&formatContext, videoState->fileName, videoState->inputFormat, &options->format) < 0) {
        ALOGE(FFPLAY_TAG, "%s avformat could not open input", __func__);
        closeReadThread(videoState, formatContext);
        return NEGATIVE(S_NOT_OPEN_INPUT);
    }

    if (msgQueue) {
        msgQueue->notifyMsg(Message::MSG_OPEN_INPUT);
    }

    if (scanAllPmtsSet) {
        av_dict_set(&options->format, SCAN_ALL_PMTS, nullptr, AV_DICT_MATCH_CASE);
    }

    if ((dictionaryEntry = av_dict_get(options->format, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
        ALOGE(FFPLAY_TAG, "%s option %s not found.", __func__, dictionaryEntry->key);
        closeReadThread(videoState, formatContext);
        return NEGATIVE_OPTION_NOT_FOUND;
    }

    videoState->formatContext = formatContext;

    if (options->generatePts) {
        formatContext->flags |= AVFMT_FLAG_GENPTS;
    }

    av_format_inject_global_side_data(formatContext);

    if (options->findStreamInfo) {
        AVDictionary **opts = setupFindStreamInfoOpts(formatContext, options->codec);
        int ret = avformat_find_stream_info(formatContext, opts);
        if (msgQueue) {
            msgQueue->notifyMsg(Message::MSG_FIND_STREAM_INFO);
        }
        for (int i = 0; i < formatContext->nb_streams; i++) {
            av_dict_free(&opts[i]);
        }
        av_freep(&opts);
        if (ret < 0) {
            ALOGD(FFPLAY_TAG, "%s %s: could not find codec parameters", __func__, videoState->fileName);
            closeReadThread(videoState, formatContext);
            return NEGATIVE(S_NOT_FIND_STREAM_INFO);
        }
    }

    if (formatContext->pb) {
        formatContext->pb->eof_reached = 0;
    }

    if (options->seekByBytes < 0) {
        options->seekByBytes = (formatContext->iformat->flags & AVFMT_TS_DISCONT) &&
                               strcmp(OGG, formatContext->iformat->name) != 0;
    }

    videoState->maxFrameDuration = (formatContext->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;

    if (!options->windowTitle && (dictionaryEntry = av_dict_get(formatContext->metadata, TITLE, nullptr, 0))) {
        options->windowTitle = av_asprintf("%s - %s", dictionaryEntry->value, options->inputFileName);
    }

    /* if seeking requested, we execute it */
    if (options->startTime != AV_NOPTS_VALUE) {
        int64_t timestamp = options->startTime;
        /* add the stream start time */
        if (formatContext->start_time != AV_NOPTS_VALUE) {
            timestamp += formatContext->start_time;
        }
        if (avformat_seek_file(formatContext, -1, INT64_MIN, timestamp, INT64_MAX, 0) < 0) {
            ALOGD(FFPLAY_TAG, "%s %s: could not seek to position %0.3f", __func__, videoState->fileName,
                  (double) timestamp / AV_TIME_BASE);
        }
    }

    videoState->realTime = isRealTime(formatContext);

    if (options->showStatus) {
        av_dump_format(formatContext, 0, videoState->fileName, 0);
    }

    for (int i = 0; i < formatContext->nb_streams; i++) {
        AVStream *stream = formatContext->streams[i];
        AVMediaType type = stream->codecpar->codec_type;
        stream->discard = AVDISCARD_ALL;
        if (type >= 0 && options->wantedStreamSpec[type] && streamIndex[type] == -1) {
            if (avformat_match_stream_specifier(formatContext, stream, options->wantedStreamSpec[type]) > 0) {
                streamIndex[type] = i;
            }
        }
    }

    for (int i = 0; i < AVMEDIA_TYPE_NB; i++) {
        if (options->wantedStreamSpec[i] && streamIndex[i] == -1) {
            ALOGD(FFPLAY_TAG, "%s Stream specifier %s does not match any stream", __func__, options->wantedStreamSpec[i]);
            streamIndex[i] = INT_MAX;
        }
    }

    streamIndex[AVMEDIA_TYPE_VIDEO] = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO,
                                                          streamIndex[AVMEDIA_TYPE_VIDEO], -1, nullptr, 0);

    streamIndex[AVMEDIA_TYPE_AUDIO] = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO,
                                                          streamIndex[AVMEDIA_TYPE_AUDIO],
                                                          streamIndex[AVMEDIA_TYPE_VIDEO],
                                                          nullptr, 0);

    streamIndex[AVMEDIA_TYPE_SUBTITLE] = av_find_best_stream(formatContext, AVMEDIA_TYPE_SUBTITLE,
                                                             streamIndex[AVMEDIA_TYPE_SUBTITLE],
                                                             (streamIndex[AVMEDIA_TYPE_AUDIO] >= 0 ?
                                                              streamIndex[AVMEDIA_TYPE_AUDIO] :
                                                              streamIndex[AVMEDIA_TYPE_VIDEO]),
                                                             nullptr, 0);

    videoState->showMode = options->showMode;

    if (streamIndex[AVMEDIA_TYPE_VIDEO] >= 0) {
        AVStream *stream = formatContext->streams[streamIndex[AVMEDIA_TYPE_VIDEO]];
        AVCodecParameters *codecParameters = stream->codecpar;
        AVRational sampleAspectRatio = av_guess_sample_aspect_ratio(formatContext, stream, nullptr);
        if (codecParameters->width && surface) {
            surface->setWindowSize(codecParameters->width, codecParameters->height, sampleAspectRatio);
        }
    }

    /* open the streams */
    if (streamIndex[AVMEDIA_TYPE_AUDIO] >= 0) {
        streamComponentOpen(streamIndex[AVMEDIA_TYPE_AUDIO]);
    } else {
        videoState->syncType = options->syncType = SYNC_TYPE_VIDEO_MASTER;
    }

    if (streamIndex[AVMEDIA_TYPE_VIDEO] >= 0) {
        ret = streamComponentOpen(streamIndex[AVMEDIA_TYPE_VIDEO]);
    }

    if (videoState->showMode == SHOW_MODE_NONE) {
        videoState->showMode = ret >= 0 ? SHOW_MODE_VIDEO : SHOW_MODE_RDFT;
    }

    if (streamIndex[AVMEDIA_TYPE_SUBTITLE] >= 0) {
        streamComponentOpen(streamIndex[AVMEDIA_TYPE_SUBTITLE]);
    }

    if (msgQueue) {
        msgQueue->notifyMsg(Message::MSG_COMPONENT_OPEN);
    }

    if (videoState->videoStreamIndex < 0 && videoState->audioStreamIndex < 0) {
        ALOGD(FFPLAY_TAG, "%s failed to open file '%s' or configure filter graph", __func__, videoState->fileName);
        closeReadThread(videoState, formatContext);
        return NEGATIVE(S_NOT_OPEN_FILE);
    }

    if (options->infiniteBuffer < 0 && videoState->realTime) {
        options->infiniteBuffer = 1;
    }

    if (videoState->videoStream && videoState->videoStream->codecpar) {
        AVCodecParameters *codecParameters = videoState->videoStream->codecpar;
        if (msgQueue) {
            msgQueue->notifyMsg(Message::MSG_VIDEO_SIZE_CHANGED, codecParameters->width, codecParameters->height);
            msgQueue->notifyMsg(Message::MSG_SAR_CHANGED, codecParameters->width, codecParameters->height);
        }
    }

    if (msgQueue) {
        msgQueue->notifyMsg(Message::MSG_PREPARED);
    }

    if (msgQueue) {
        msgQueue->notifyMsg(Message::REQ_START);
    }

    for (;;) {

        if (videoState->abortRequest) {
            break;
        }

        if (videoState->paused != videoState->lastPaused) {
            videoState->lastPaused = videoState->paused;
            if (videoState->paused) {
                videoState->readPauseReturn = av_read_pause(formatContext);
            } else {
                av_read_play(formatContext);
            }
        }

        if (videoState->seekReq) {
            int64_t seekTarget = videoState->seekPos;
            int64_t seekMin = videoState->seekRel > 0 ? (seekTarget - videoState->seekRel + 2) : INT64_MIN;
            int64_t seekMax = videoState->seekRel < 0 ? (seekTarget - videoState->seekRel - 2) : INT64_MAX;
            if (avformat_seek_file(videoState->formatContext, -1, seekMin, seekTarget, seekMax, videoState->seekFlags) < 0) {
                ALOGD(FFPLAY_TAG, "%s %s: error while seeking", __func__, videoState->formatContext->url);
            } else {
                if (videoState->audioStreamIndex >= 0) {
                    videoState->audioPacketQueue.flush();
                    videoState->audioPacketQueue.put(&flushPacket);
                }
                if (videoState->subtitleStreamIndex >= 0) {
                    videoState->subtitlePacketQueue.flush();
                    videoState->subtitlePacketQueue.put(&flushPacket);
                }
                if (videoState->videoStreamIndex >= 0) {
                    videoState->videoPacketQueue.flush();
                    videoState->videoPacketQueue.put(&flushPacket);
                }
                if (videoState->seekFlags & AVSEEK_FLAG_BYTE) {
                    videoState->exitClock.setClock(NAN, 0);
                } else {
                    videoState->exitClock.setClock(seekTarget / (double) AV_TIME_BASE, 0);
                }
            }
            videoState->seekReq = 0;
            videoState->queueAttachmentsReq = 1;
            videoState->eof = 0;
            if (videoState->paused) {
                stepToNextFrame();
            }
        }

        // https://segmentfault.com/a/1190000018373504?utm_source=tag-newest
        if (videoState->queueAttachmentsReq) {
            if (videoState->videoStream && videoState->videoStream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
                AVPacket copy = {nullptr};
                if ((av_packet_ref(&copy, &videoState->videoStream->attached_pic)) < 0) {
                    closeReadThread(videoState, formatContext);
                    return NEGATIVE(S_NOT_ATTACHED_PIC);
                }
                videoState->videoPacketQueue.put(&copy);
                videoState->videoPacketQueue.putNullPacket(videoState->videoStreamIndex);
            }
            videoState->queueAttachmentsReq = 0;
        }

        /* if the packetQueue are full, no need to read more */
        bool cond1 = options->infiniteBuffer < 1;
        bool cond2 = (videoState->audioPacketQueue.memorySize + videoState->videoPacketQueue.memorySize + videoState->subtitlePacketQueue.memorySize) > MAX_QUEUE_SIZE;
        int cond31 = streamHasEnoughPackets(videoState->audioStream, videoState->audioStreamIndex, &videoState->audioPacketQueue);
        int cond32 = streamHasEnoughPackets(videoState->videoStream, videoState->videoStreamIndex, &videoState->videoPacketQueue);
        int cond33 = streamHasEnoughPackets(videoState->subtitleStream, videoState->subtitleStreamIndex, &videoState->subtitlePacketQueue);
        bool cond3 = cond31 && cond32 && cond33;
        if (cond1 && (cond2 || cond3)) {
            /* wait 10 ms */
            waitMutex->mutexLock();
            videoState->continueReadThread->condWaitTimeout(waitMutex, 10);
            waitMutex->mutexUnLock();
            continue;
        }

        // 未暂停
        bool notPaused = !videoState->paused;
        // 未初始化音频流 或者 解码结束 同时 无可用帧
        bool audioSeekCond = !videoState->audioStream || (videoState->audioDecoder.finished == videoState->audioPacketQueue.serial && videoState->audioFrameQueue.numberRemaining() == 0);
        // 未初始化视频流 或者 解码结束 同时 无可用帧
        bool videoSeekCond = !videoState->videoStream || (videoState->videoDecoder.finished == videoState->videoPacketQueue.serial && videoState->videoFrameQueue.numberRemaining() == 0);
        if (notPaused && audioSeekCond && videoSeekCond) {
            if (options->loop != 1 && (!options->loop || --options->loop)) {
                streamSeek(options->startTime != AV_NOPTS_VALUE ? options->startTime : 0, 0, 0);
            } else if (options->autoExit) {
                return NEGATIVE(NEGATIVE_EOF);
            }
        }

        // 读取帧
        ret = av_read_frame(formatContext, packet);

        if (ret < 0) {
            // 0 if OK, < 0 on error or end of file
            if ((ret == AVERROR_EOF || avio_feof(formatContext->pb)) && !videoState->eof) {
                if (videoState->videoStreamIndex >= 0) {
                    videoState->videoPacketQueue.putNullPacket(videoState->videoStreamIndex);
                }
                if (videoState->audioStreamIndex >= 0) {
                    videoState->audioPacketQueue.putNullPacket(videoState->audioStreamIndex);
                }
                if (videoState->subtitleStreamIndex >= 0) {
                    videoState->subtitlePacketQueue.putNullPacket(videoState->subtitleStreamIndex);
                }
                videoState->eof = 1;
            }
            if (formatContext->pb && formatContext->pb->error) {
                closeReadThread(videoState, formatContext);
                break;
            }
            waitMutex->mutexLock();
            videoState->continueReadThread->condWaitTimeout(waitMutex, 10);
            waitMutex->mutexUnLock();
            continue;
        } else {
            videoState->eof = 0;
        }

        /* check if packet videoState in play range specified by user, then packetQueue, otherwise discard */
        streamStartTime = formatContext->streams[packet->stream_index]->start_time;
        // https://baike.baidu.com/item/PTS/13977433
        packetTimestamp = (packet->pts == AV_NOPTS_VALUE) ? packet->dts : packet->pts;
        int64_t diffTimestamp = packetTimestamp - (streamStartTime != AV_NOPTS_VALUE ? streamStartTime : 0);

        double _diffTime = diffTimestamp * av_q2d(formatContext->streams[packet->stream_index]->time_base);
        double _startTime = (double) (options->startTime != AV_NOPTS_VALUE ? options->startTime : 0) / 1000000;
        double _duration = (double) options->duration / 1000000;

        packetInPlayRange = (options->duration == AV_NOPTS_VALUE) || (_diffTime - _startTime <= _duration);

        if (packet->stream_index == videoState->audioStreamIndex && packetInPlayRange) {
            videoState->audioPacketQueue.put(packet);
//            ALOGD(FFPLAY_TAG,"%s audio memorySize=%d serial=%d packetSize=%d duration=%lld abortRequest=%d", __func__,
//                  videoState->audioPacketQueue.memorySize,
//                  videoState->audioPacketQueue.serial,
//                  videoState->audioPacketQueue.packetSize,
//                  duration,
//                  videoState->audioPacketQueue.abortRequest);
        } else if (packet->stream_index == videoState->videoStreamIndex && packetInPlayRange &&
                   !(videoState->videoStream->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
            videoState->videoPacketQueue.put(packet);
//            ALOGD(FFPLAY_TAG,"%s video memorySize=%d serial=%d packetSize=%d duration=%lld abortRequest=%d", __func__,
//                  videoState->videoPacketQueue.memorySize,
//                  videoState->videoPacketQueue.serial,
//                  videoState->videoPacketQueue.packetSize,
//                  duration,
//                  videoState->videoPacketQueue.abortRequest);
        } else if (packet->stream_index == videoState->subtitleStreamIndex && packetInPlayRange) {
            videoState->subtitlePacketQueue.put(packet);
//            ALOGD(FFPLAY_TAG,"%s subtitle memorySize=%d serial=%d packetSize=%d duration=%lld abortRequest=%d", __func__,
//                  videoState->subtitlePacketQueue.memorySize,
//                  videoState->subtitlePacketQueue.serial,
//                  videoState->subtitlePacketQueue.packetSize,
//                  duration,
//                  videoState->subtitlePacketQueue.abortRequest);
        } else {
            av_packet_unref(packet);
        }
    } // for end
    return POSITIVE;
}

void FFPlay::closeReadThread(const VideoState *is, AVFormatContext *&formatContext) const {
    ALOGD(FFPLAY_TAG, __func__);

    if (formatContext && !is->formatContext) {
        avformat_close_input(&formatContext);
    }
}

int FFPlay::isRealTime(AVFormatContext *s) {
    ALOGD(FFPLAY_TAG, __func__);

    if (!strcmp(s->iformat->name, FORMAT_RTP) ||
        !strcmp(s->iformat->name, FORMAT_RTSP) ||
        !strcmp(s->iformat->name, FORMAT_SDP)) {
        return POSITIVE;
    }
    if (s->pb &&
        (!strncmp(s->url, URL_FORMAT_RTP, 4) || !strncmp(s->url, URL_FORMAT_UDP, 4))) {
        return POSITIVE;
    }
    return NEGATIVE(S_ERROR);
}

static int innerVideoThread(void *arg) {
    FFPlay *play = static_cast<FFPlay *>(arg);
    if (play) {
        return play->videoThread();
    }
    return NEGATIVE(S_ERROR);
}

static int innerSubtitleThread(void *arg) {
    FFPlay *play = static_cast<FFPlay *>(arg);
    if (play) {
        return play->subtitleThread();
    }
    return NEGATIVE(S_ERROR);
}

static int innerAudioThread(void *arg) {
    FFPlay *play = static_cast<FFPlay *>(arg);
    if (play) {
        return play->audioThread();
    }
    return NEGATIVE(S_ERROR);
}

/* open a given stream. Return 0 if OK */
int FFPlay::streamComponentOpen(int streamIndex) {
    ALOGD(FFPLAY_TAG, "%s streamIndex=%d", __func__, streamIndex);

    AVFormatContext *formatContext = videoState->formatContext;
    AVCodecContext *codecContext;
    AVCodec *codec;
    const char *forcedCodecName = nullptr;
    AVDictionary *opts = nullptr;
    AVDictionaryEntry *t = nullptr;
    int sampleRate, nbChannels;
    int64_t channelLayout;
    int streamLowres = options->lowres;

    if (streamIndex < 0 || streamIndex >= formatContext->nb_streams) {
        return NEGATIVE(S_INVALID_STREAM_INDEX);
    }

    codecContext = avcodec_alloc_context3(nullptr);
    if (!codecContext) {
        return NEGATIVE(S_NOT_MEMORY);
    }

    if (avcodec_parameters_to_context(codecContext, formatContext->streams[streamIndex]->codecpar) < 0) {
        avcodec_free_context(&codecContext);
        return NEGATIVE(S_NOT_CODEC_PARAMS_CONTEXT);
    }

    codecContext->pkt_timebase = formatContext->streams[streamIndex]->time_base;

    codec = avcodec_find_decoder(codecContext->codec_id);

    switch (codecContext->codec_type) {
        case AVMEDIA_TYPE_AUDIO   :
            videoState->lastAudioStreamIndex = streamIndex;
            forcedCodecName = options->forceAudioCodecName;
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            videoState->lastSubtitleStreamIndex = streamIndex;
            forcedCodecName = options->forceSubtitleCodecName;
            break;
        case AVMEDIA_TYPE_VIDEO   :
            videoState->lastVideoStreamIndex = streamIndex;
            forcedCodecName = options->forceVideoCodecName;
            break;
        case AVMEDIA_TYPE_UNKNOWN:
            break;
        case AVMEDIA_TYPE_DATA:
            break;
        case AVMEDIA_TYPE_ATTACHMENT:
            break;
        case AVMEDIA_TYPE_NB:
            break;
    }

    if (forcedCodecName) {
        codec = avcodec_find_decoder_by_name(forcedCodecName);
    }

    if (!codec) {
        if (forcedCodecName) {
            ALOGD(FFPLAY_TAG, "%s No codec could be found with name '%s'", __func__, forcedCodecName);
        } else {
            ALOGD(FFPLAY_TAG, "%sNo decoder could be found for codec %s", __func__, avcodec_get_name(codecContext->codec_id));
        }
        avcodec_free_context(&codecContext);
        return NEGATIVE(S_NOT_FOUND_CODER);
    }

    codecContext->codec_id = codec->id;

    if (streamLowres > codec->max_lowres) {
        ALOGD(FFPLAY_TAG, "%s The maximum value for lowres supported by the decoder is %d", __func__,
              codec->max_lowres);
        streamLowres = codec->max_lowres;
    }
    codecContext->lowres = streamLowres;

    if (options->fast) {
        codecContext->flags2 |= AV_CODEC_FLAG2_FAST;
    }

    opts = filter_codec_opts(options->codec, codecContext->codec_id, formatContext, formatContext->streams[streamIndex], codec);

    if (!av_dict_get(opts, "threads", nullptr, 0)) {
        av_dict_set(&opts, "threads", "auto", 0);
    }
    if (streamLowres) {
        av_dict_set_int(&opts, "lowres", streamLowres, 0);
    }
    if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO || codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
        av_dict_set(&opts, "refcounted_frames", "1", 0);
    }

    if (avcodec_open2(codecContext, codec, &opts) < 0) {
        avcodec_free_context(&codecContext);
        return NEGATIVE(S_NOT_OPEN_DECODEC);
    }

    if ((t = av_dict_get(opts, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
        ALOGE(FFPLAY_TAG, "%s Option %s not found.", __func__, t->key);
        return NEGATIVE(S_NOT_FOUND_OPTION);
    }

    videoState->eof = 0;

    formatContext->streams[streamIndex]->discard = AVDISCARD_DEFAULT;

    switch (codecContext->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            videoState->audioStreamIndex = streamIndex;
            videoState->audioStream = formatContext->streams[streamIndex];
            videoState->audioDecoder.decoderInit(codecContext, &videoState->audioPacketQueue, videoState->continueReadThread);
            if ((videoState->formatContext->iformat->flags & (AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK)) && !videoState->formatContext->iformat->read_seek) {
                videoState->audioDecoder.startPts = videoState->audioStream->start_time;
                videoState->audioDecoder.startPtsTb = videoState->audioStream->time_base;
            }
            if (videoState->audioDecoder.decoderStart("ADecode", innerAudioThread, this) < 0) {
                av_dict_free(&opts);
                return NEGATIVE(S_NOT_AUDIO_DECODE_START);
            }
            break;
        case AVMEDIA_TYPE_VIDEO:
            videoState->videoStreamIndex = streamIndex;
            videoState->videoStream = formatContext->streams[streamIndex];
            videoState->videoDecoder.decoderInit(codecContext, &videoState->videoPacketQueue, videoState->continueReadThread);
            if (videoState->videoDecoder.decoderStart("VDecode", innerVideoThread, this) < 0) {
                av_dict_free(&opts);
                return NEGATIVE(S_NOT_VIDEO_DECODE_START);
            }
            videoState->queueAttachmentsReq = 1;
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            videoState->subtitleStreamIndex = streamIndex;
            videoState->subtitleStream = formatContext->streams[streamIndex];
            videoState->subtitleDecoder.decoderInit(codecContext, &videoState->subtitlePacketQueue, videoState->continueReadThread);
            if (videoState->subtitleDecoder.decoderStart("SDecode", innerSubtitleThread, this) < 0) {
                av_dict_free(&opts);
                return NEGATIVE(S_NOT_SUBTITLE_DECODE_START);
            }
            break;
        default:
            break;
    }
    return POSITIVE;
}

int FFPlay::streamHasEnoughPackets(AVStream *stream, int streamIndex, PacketQueue *packetQueue) {
    return streamIndex < 0 || packetQueue->abortRequest || (stream->disposition & AV_DISPOSITION_ATTACHED_PIC) || (packetQueue->packetSize > MIN_FRAMES && (!packetQueue->duration || av_q2d(stream->time_base) * packetQueue->duration > 1.0));
}

int FFPlay::streamComponentClose(AVStream *stream, int streamIndex) {
    AVFormatContext *ic = videoState->formatContext;
    AVCodecParameters *codecParameters;

    if (streamIndex < 0 || streamIndex >= ic->nb_streams) {
        return NEGATIVE(S_INVALID_STREAM_INDEX);
    }

    codecParameters = ic->streams[streamIndex]->codecpar;

    switch (codecParameters->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            videoState->audioDecoder.decoderAbort(&videoState->audioFrameQueue);
            videoState->audioDecoder.decoderDestroy();
            swr_free(&videoState->swrContext);
            av_freep(&videoState->audioBuf1);
            videoState->audioBuf1Size = 0;
            videoState->audioBuf = nullptr;
            if (videoState->rdft) {
                av_rdft_end(videoState->rdft);
                av_freep(&videoState->rdft_data);
                videoState->rdft = nullptr;
                videoState->rdft_bits = 0;
            }
            break;
        case AVMEDIA_TYPE_VIDEO:
            videoState->videoDecoder.decoderAbort(&videoState->videoFrameQueue);
            videoState->videoDecoder.decoderDestroy();
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            videoState->subtitleDecoder.decoderAbort(&videoState->subtitleFrameQueue);
            videoState->subtitleDecoder.decoderDestroy();
            break;
        default:
            break;
    }

    ic->streams[streamIndex]->discard = AVDISCARD_ALL;

    switch (codecParameters->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            videoState->audioStream = nullptr;
            videoState->audioStreamIndex = -1;
            break;
        case AVMEDIA_TYPE_VIDEO:
            videoState->videoStream = nullptr;
            videoState->videoStreamIndex = -1;
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            videoState->subtitleStream = nullptr;
            videoState->subtitleStreamIndex = -1;
            break;
        default:
            break;
    }

    return POSITIVE;
}

/* get the current master clock value */
double FFPlay::getMasterClock() {
    double val;
    switch (getMasterSyncType()) {
        case SYNC_TYPE_VIDEO_MASTER:
            val = videoState->videoClock.getClock();
            break;
        case SYNC_TYPE_AUDIO_MASTER:
            val = videoState->audioClock.getClock();
            break;
        default:
            val = videoState->exitClock.getClock();
            break;
    }
    return val;
}

int FFPlay::getMasterSyncType() {
    if (videoState && videoState->syncType == SYNC_TYPE_VIDEO_MASTER) {
        if (videoState->videoStream) {
            return SYNC_TYPE_VIDEO_MASTER;
        } else {
            return SYNC_TYPE_AUDIO_MASTER;
        }
    } else if (videoState && videoState->syncType == SYNC_TYPE_AUDIO_MASTER) {
        if (videoState->audioStream) {
            return SYNC_TYPE_AUDIO_MASTER;
        } else {
            return SYNC_TYPE_EXTERNAL_CLOCK;
        }
    } else {
        return SYNC_TYPE_EXTERNAL_CLOCK;
    }
}

void FFPlay::stepToNextFrame() {
    // TODO
}

void FFPlay::streamSeek(int64_t pos, int64_t rel, int seek_by_bytes) {
    /* seek in the stream */
    if (!videoState->seekReq) {
        videoState->seekPos = pos;
        videoState->seekRel = rel;
        videoState->seekFlags &= ~AVSEEK_FLAG_BYTE;
        if (seek_by_bytes) {
            videoState->seekFlags |= AVSEEK_FLAG_BYTE;
        }
        videoState->seekReq = 1;
        videoState->continueReadThread->condSignal();
    }
}

int FFPlay::videoThread() {
    ALOGD(FFPLAY_TAG, __func__);

    VideoState *is = videoState;
    AVFrame *frame = av_frame_alloc();
    double pts;
    double duration;
    int ret;
    AVRational timeBase = is->videoStream->time_base;
    AVRational frameRate = av_guess_frame_rate(is->formatContext, is->videoStream, nullptr);

    if (!frame) {
        return NEGATIVE(S_NOT_MEMORY);
    }

    for (;;) {
        ALOGI(FFPLAY_TAG, "%s prepare get video frame", __func__);
        ret = getVideoFrame(frame);
        if (ret < 0) {
            av_frame_free(&frame);
            ALOGE(FFPLAY_TAG, "%s not get video frame", __func__);
            return NEGATIVE(S_NOT_GET_VIDEO_FRAME);
        }
        if (ret == NEGATIVE_EOF) {
            continue;
        }

        duration = (frameRate.num && frameRate.den ? av_q2d((AVRational) {frameRate.den, frameRate.num}) : 0);
        pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(timeBase);
        ret = queuePicture(frame, pts, duration, frame->pkt_pos, is->videoDecoder.packetSerial);
        av_frame_unref(frame);

        if (ret < 0) {
            av_frame_free(&frame);
            ALOGE(FFPLAY_TAG, "%s not queue picture", __func__);
            return NEGATIVE(S_NOT_QUEUE_PICTURE);
        }
    }

    ALOGD(FFPLAY_TAG, "%s end", __func__);
    return POSITIVE;
}

int FFPlay::subtitleThread() {
    ALOGD(FFPLAY_TAG, __func__);
    return POSITIVE;
}

int FFPlay::audioThread() {
    ALOGD(FFPLAY_TAG, __func__);
    return POSITIVE;
}

int FFPlay::getVideoFrame(AVFrame *frame) {
    ALOGD(FFPLAY_TAG, __func__);

    int gotPicture;

    if ((gotPicture = decoderDecodeFrame(&videoState->videoDecoder, frame, nullptr)) < 0) {
        return NEGATIVE(S_NOT_DECODE_FRAME);
    }

    if (gotPicture) {
        double dpts = NAN;

        if (frame->pts != AV_NOPTS_VALUE) {
            dpts = av_q2d(videoState->videoStream->time_base) * frame->pts;
        }

        frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(videoState->formatContext, videoState->videoStream, frame);

        if (options->dropFrameWhenSlow > 0 || (options->dropFrameWhenSlow && getMasterSyncType() != SYNC_TYPE_VIDEO_MASTER)) {
            if (frame->pts != AV_NOPTS_VALUE) {
                double diff = dpts - getMasterClock();
                if (!isnan(diff) && fabs(diff) < NOSYNC_THRESHOLD &&
                    diff - videoState->frameLastFilterDelay < 0 &&
                    videoState->videoDecoder.packetSerial == videoState->videoClock.serial &&
                    videoState->videoPacketQueue.packetSize) {
                    videoState->frameDropsEarly++;
                    av_frame_unref(frame);
                    gotPicture = 0;
                }
            }
        }
    }

    return gotPicture;
}

int FFPlay::decoderDecodeFrame(Decoder *decoder, AVFrame *frame, AVSubtitle *subtitle) {
    ALOGD(FFPLAY_TAG, __func__);

    int ret = AVERROR(EAGAIN);

    for (;;) {
        AVPacket packet;

        if (decoder->packetQueue->serial == decoder->packetSerial) {
            // 接收一帧解码后的数据
            do {
                if (decoder->packetQueue->abortRequest) {
                    return NEGATIVE(S_ABORT_REQUEST);
                }
                switch (decoder->codecContext->codec_type) {
                    case AVMEDIA_TYPE_VIDEO:
                        ret = avcodec_receive_frame(decoder->codecContext, frame);
                        if (ret >= 0) {
                            if (options->decoderReorderPts == -1) {
                                frame->pts = frame->best_effort_timestamp;
                            } else if (!options->decoderReorderPts) {
                                frame->pts = frame->pkt_dts;
                            }
                        }
                        break;
                    case AVMEDIA_TYPE_AUDIO:
                        ret = avcodec_receive_frame(decoder->codecContext, frame);
                        if (ret >= 0) {
                            AVRational tb = (AVRational) {1, frame->sample_rate};
                            if (frame->pts != AV_NOPTS_VALUE) {
                                frame->pts = av_rescale_q(frame->pts, decoder->codecContext->pkt_timebase, tb);
                            } else if (decoder->nextPts != AV_NOPTS_VALUE) {
                                frame->pts = av_rescale_q(decoder->nextPts, decoder->nextPtsTb, tb);
                            }
                            if (frame->pts != AV_NOPTS_VALUE) {
                                decoder->nextPts = frame->pts + frame->nb_samples;
                                decoder->nextPtsTb = tb;
                            }
                        }
                        break;
                    case AVMEDIA_TYPE_UNKNOWN:
                        break;
                    case AVMEDIA_TYPE_DATA:
                        break;
                    case AVMEDIA_TYPE_SUBTITLE:
                        break;
                    case AVMEDIA_TYPE_ATTACHMENT:
                        break;
                    case AVMEDIA_TYPE_NB:
                        break;
                }
                if (ret == AVERROR_EOF) {
                    decoder->finished = decoder->packetSerial;
                    avcodec_flush_buffers(decoder->codecContext);
                    return NEGATIVE_EOF;
                }
                if (ret >= 0) {
                    return POSITIVE;
                }
            } while (ret != AVERROR(EAGAIN));
        }

        do {
            if (decoder->packetQueue->packetSize == 0) {
                decoder->emptyQueueCond->condSignal();
            }
            if (decoder->packetPending) {
                av_packet_move_ref(&packet, &decoder->packet);
                decoder->packetPending = 0;
            } else {
                if (decoder->packetQueue->get(&packet, 1, &decoder->packetSerial) < 0) {
                    return NEGATIVE(S_NOT_GET_PACKET_QUEUE);
                }
            }
        } while (decoder->packetQueue->serial != decoder->packetSerial);

        if (packet.data == flushPacket.data) {
            avcodec_flush_buffers(decoder->codecContext);
            decoder->finished = 0;
            decoder->nextPts = decoder->startPts;
            decoder->nextPtsTb = decoder->startPtsTb;
        } else {
            if (decoder->codecContext->codec_type == AVMEDIA_TYPE_SUBTITLE) {
                int gotFrame = 0;
                ret = avcodec_decode_subtitle2(decoder->codecContext, subtitle, &gotFrame, &packet);
                if (ret < 0) {
                    ret = NEGATIVE(EAGAIN);
                } else {
                    if (gotFrame && !packet.data) {
                        decoder->packetPending = 1;
                        av_packet_move_ref(&decoder->packet, &packet);
                    }
                    ret = gotFrame ? POSITIVE : (packet.data ? NEGATIVE(EAGAIN) : NEGATIVE_EOF);
                }
            } else {
                if (avcodec_send_packet(decoder->codecContext, &packet) == AVERROR(EAGAIN)) {
                    ALOGE(FFPLAY_TAG, "%s Receive_frame and send_packet both returned EAGAIN, which is an API violation.", __func__);
                    decoder->packetPending = 1;
                    av_packet_move_ref(&decoder->packet, &packet);
                }
            }
            av_packet_unref(&packet);
        }
    }

    return ret;
}

int FFPlay::queuePicture(AVFrame *srcFrame, double pts, double duration, int64_t pos, int serial) {
    ALOGD(FFPLAY_TAG, "%s pts=%lf duration=%lf pos=%lld serial=%d", __func__, pts, duration, pos, serial);

    Frame *vp;

    if (!(vp = videoState->videoFrameQueue.peekWritable())) {
        return NEGATIVE(S_NOT_FRAME_WRITEABLE);
    }

    vp->sampleAspectRatio = srcFrame->sample_aspect_ratio;
    vp->uploaded = 0;

    vp->width = srcFrame->width;
    vp->height = srcFrame->height;
    vp->format = srcFrame->format;

    vp->pts = pts;
    vp->duration = duration;
    vp->pos = pos;
    vp->serial = serial;

    av_frame_move_ref(vp->frame, srcFrame);
    videoState->videoFrameQueue.push();
    return POSITIVE;
}

int FFPlay::refresh() {
    ALOGD(FFPLAY_TAG, "---------------- start");
    ALOGD(FFPLAY_TAG, "%s while remainingTime = %lf paused = %d forceRefresh = %d", __func__, remainingTime, videoState->paused, videoState->forceRefresh);
    if (remainingTime > 0.0) {
        av_usleep(static_cast<unsigned int>((int64_t) (remainingTime * 1000000.0)));
    }
    remainingTime = REFRESH_RATE;
    if (videoState->showMode != SHOW_MODE_NONE && (!videoState->paused || videoState->forceRefresh)) {
        refreshVideo(&remainingTime);
    }
    ALOGD(FFPLAY_TAG, "---------------- end");
    return POSITIVE;
}

void FFPlay::checkExternalClockSpeed() {
    if ((videoState->videoStreamIndex >= 0 && videoState->videoPacketQueue.packetSize <= EXTERNAL_CLOCK_MIN_FRAMES) ||
        (videoState->audioStreamIndex >= 0 && videoState->audioPacketQueue.packetSize <= EXTERNAL_CLOCK_MIN_FRAMES)) {
        videoState->exitClock.setClockSpeed(FFMAX(EXTERNAL_CLOCK_SPEED_MIN, videoState->exitClock.speed - EXTERNAL_CLOCK_SPEED_STEP));
    } else if ((videoState->videoStreamIndex < 0 || videoState->videoPacketQueue.packetSize > EXTERNAL_CLOCK_MAX_FRAMES) &&
               (videoState->audioStreamIndex < 0 || videoState->audioPacketQueue.packetSize > EXTERNAL_CLOCK_MAX_FRAMES)) {
        videoState->exitClock.setClockSpeed(FFMIN(EXTERNAL_CLOCK_SPEED_MAX, videoState->exitClock.speed + EXTERNAL_CLOCK_SPEED_STEP));
    } else {
        double speed = videoState->exitClock.speed;
        if (speed != 1.0) {
            videoState->exitClock.setClockSpeed(speed + EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / fabs(1.0 - speed));
        }
    }
}

/* called to display each frame */
void FFPlay::refreshVideo(double *remainingTime) {
    ALOGD(FFPLAY_TAG, "%s", __func__);
    double time;
    Frame *sp, *sp2;

    if (!videoState->paused && getMasterSyncType() == SYNC_TYPE_EXTERNAL_CLOCK && videoState->realTime) {
        checkExternalClockSpeed();
    }

    if (videoState->showMode != SHOW_MODE_VIDEO && videoState->audioStream) {
        time = av_gettime_relative() / 1000000.0;
        if (videoState->forceRefresh || (videoState->lastVisTime + options->rdftSpeed) < time) {
            displayVideo();
            videoState->lastVisTime = time;
        }
        *remainingTime = FFMIN(*remainingTime, videoState->lastVisTime + options->rdftSpeed - time);
    }

    if (videoState->videoStream) {
        retry:
        if (videoState->videoFrameQueue.numberRemaining() == 0) {
            ALOGD(FFPLAY_TAG, "nothing to do, no picture to display in the queue");
        } else {
            double lastDuration, duration, delay;
            Frame *currentFrame, *lastFrame;

            /* dequeue the picture */
            lastFrame = videoState->videoFrameQueue.peekLast();
            currentFrame = videoState->videoFrameQueue.peek();

            ALOGD(FFPLAY_TAG, "peek frame serial = %d list serial = %d ", currentFrame->serial, videoState->videoPacketQueue.serial);
            if (currentFrame->serial != videoState->videoPacketQueue.serial) {
                videoState->videoFrameQueue.next();
                ALOGD(FFPLAY_TAG, "goto retry");
                goto retry;
            }

            ALOGD(FFPLAY_TAG, "peek frame serial = %d last frame serial = %d ", currentFrame->serial, lastFrame->serial);
            if (lastFrame->serial != currentFrame->serial) {
                videoState->frameTimer = av_gettime_relative() / 1000000.0;
                ALOGD(FFPLAY_TAG, "update frameTimer = %fd ", videoState->frameTimer);
            }

            if (videoState->paused) {
                ALOGD(FFPLAY_TAG, "goto display");
                goto display;
            }

            /* compute nominal lastDuration */
            lastDuration = frameDuration(lastFrame, currentFrame);
            delay = computeTargetDelay(lastDuration);

            time = av_gettime_relative() / 1000000.0;
            double frameTimerDelay = videoState->frameTimer + delay;
            ALOGD(FFPLAY_TAG, "relative time = %f frameTimerDelay = %f ", time, frameTimerDelay);
            if (time < frameTimerDelay) {
                *remainingTime = FFMIN(videoState->frameTimer + delay - time, *remainingTime);
                ALOGD(FFPLAY_TAG, "goto display");
                goto display;
            }

            videoState->frameTimer += delay;
            ALOGD(FFPLAY_TAG, "frameTimer = %f ", videoState->frameTimer);
            if (delay > 0 && (time - videoState->frameTimer) > SYNC_THRESHOLD_MAX) {
                videoState->frameTimer = time;
                ALOGD(FFPLAY_TAG, "force frameTimer = %f ", videoState->frameTimer);
            }

            videoState->videoFrameQueue.mutex->mutexLock();
            if (!isnan(currentFrame->pts)) {
                updateVideoClockPts(currentFrame->pts, currentFrame->pos, currentFrame->serial);
            }
            videoState->videoFrameQueue.mutex->mutexUnLock();

            if (videoState->videoFrameQueue.numberRemaining() > 1) {
                Frame *nextFrame = videoState->videoFrameQueue.peekNext();
                duration = frameDuration(currentFrame, nextFrame);
                ALOGD(FFPLAY_TAG, "next frame duration = %lf ", duration);
                if (!videoState->step && (options->dropFrameWhenSlow > 0 || (options->dropFrameWhenSlow && getMasterSyncType() != SYNC_TYPE_VIDEO_MASTER)) && time > (videoState->frameTimer + duration)) {
                    videoState->frameDropsLate++;
                    videoState->videoFrameQueue.next();
                    ALOGD(FFPLAY_TAG, "goto retry");
                    goto retry;
                }
            }

            // TODO subtitle

            videoState->videoFrameQueue.next();
            videoState->forceRefresh = 1;

            if (videoState->step && !videoState->paused) {
                // stream_toggle_pause();
                // TODO
            }
        }

        display:
        /* display picture */
        if (videoState->forceRefresh && videoState->showMode == SHOW_MODE_VIDEO && videoState->videoFrameQueue.readIndexShown) {
            displayVideo();
        }
    } else {
        ALOGD(FFPLAY_TAG, "not video streaming");
    }
    videoState->forceRefresh = 0;
}

double FFPlay::frameDuration(Frame *currentFrame, Frame *nextFrame) {
    if (currentFrame->serial == nextFrame->serial) {
        double duration = nextFrame->pts - currentFrame->pts;
        ALOGD(FFPLAY_TAG, "%s duration = %f maxFrameDuration = %f", __func__, duration, videoState->maxFrameDuration);
        if (isnan(duration) || duration <= 0 || duration > videoState->maxFrameDuration) {
            return currentFrame->duration;
        } else {
            return duration;
        }
    } else {
        return 0.0;
    }
}

/* display the current picture, if any */
void FFPlay::displayVideo() {
    ALOGD(FFPLAY_TAG, __func__);
    if (!videoState->width) {
        displayWindow();
    }
    if (videoState->audioStream && videoState->showMode != SHOW_MODE_VIDEO) {
        displayVideoAudio();
    } else if (videoState->videoStream) {
        displayVideoImage();
    }
}

double FFPlay::computeTargetDelay(double delay) {
    ALOGD(FFPLAY_TAG, "%s delay = %0.3f ", __func__, delay);
    double syncThreshold, diff = 0;
    /* update delay to follow master synchronisation source */
    if (getMasterSyncType() != SYNC_TYPE_VIDEO_MASTER) {
        /* if video is slave, we try to correct big delays byduplicating or deleting a frame */
        double videoClock = videoState->videoClock.getClock();
        double masterClock = getMasterClock();

        diff = videoClock - masterClock;

        ALOGD(FFPLAY_TAG, "videoClock = %lf masterClock = %lf ", videoClock, masterClock);

        // skip or repeat frame. We take into account the delay to compute the threshold. I still don't know
        // if it is the best guess */
        // 0.04 ~ 0.1
        syncThreshold = FFMAX(SYNC_THRESHOLD_MIN, FFMIN(SYNC_THRESHOLD_MAX, delay));

        ALOGD(FFPLAY_TAG, "diff = %lf syncThreshold[0.04,0.1] = %lf ", diff, syncThreshold);

        if (!isnan(diff) && fabs(diff) < videoState->maxFrameDuration) {
            if (diff <= -syncThreshold) {
                delay = FFMAX(0, delay + diff);
            } else if (diff >= syncThreshold && delay > SYNC_FRAMEDUP_THRESHOLD) {
                delay = delay + diff;
            } else if (diff >= syncThreshold) {
                delay = 2 * delay;
            }
        }
    }
    ALOGD(FFPLAY_TAG, "%s result delay = %0.3f ", __func__, delay);
    return delay;
}

/* update current video pts */
void FFPlay::updateVideoClockPts(double pts, int64_t pos, int serial) {
    videoState->videoClock.setClock(pts, serial);
    syncClockToSlave(&videoState->exitClock, &videoState->videoClock);
}

void FFPlay::syncClockToSlave(Clock *c, Clock *slave) {
    double clock = c->getClock();
    double slaveClock = slave->getClock();
    if (!isnan(slaveClock) && (isnan(clock) || fabs(clock - slaveClock) > NOSYNC_THRESHOLD)) {
        c->setClock(slaveClock, slave->serial);
    }
}

int FFPlay::displayWindow() {
    ALOGD(FFPLAY_TAG, __func__);
    int w, h;
    if (options->screenWidth) {
        w = options->screenWidth;
        h = options->screenHeight;
    } else {
        w = options->defaultWidth;
        h = options->defaultHeight;
    }
    if (!options->windowTitle) {
        options->windowTitle = options->inputFileName;
    }
    videoState->width = w;
    videoState->height = h;
    if (surface) {
        surface->displayWindow(w, h);
    }
    return POSITIVE;
}


void FFPlay::displayVideoImage() {
    ALOGD(FFPLAY_TAG, __func__);
    if (surface) {
        surface->displayVideoImage();
    }
}

Surface *FFPlay::getSurface() const {
    return surface;
}

Stream *FFPlay::getStream() const {
    return stream;
}

Audio *FFPlay::getAudio() const {
    return audio;
}

VideoState *FFPlay::getVideoState() const {
    return videoState;
}

void FFPlay::displayVideoAudio() {

}

Options *FFPlay::getOptions() const {
    return options;
}

void FFPlay::setOptions(Options *options) {
    FFPlay::options = options;
}
