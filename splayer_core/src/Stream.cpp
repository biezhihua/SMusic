#include "Stream.h"

static int innerReadThread(void *arg) {
    auto *play = static_cast<Stream *>(arg);
    if (play) {
        return play->readThread();
    }
    return POSITIVE;
}

static int innerVideoThread(void *arg) {
    auto *play = static_cast<Stream *>(arg);
    if (play) {
        return play->videoThread();
    }
    return NEGATIVE(S_ERROR);
}

static int innerAudioThread(void *arg) {
    auto *play = static_cast<Stream *>(arg);
    if (play) {
        return play->audioThread();
    }
    return NEGATIVE(S_ERROR);
}

static int innerSubtitleThread(void *arg) {
    auto *play = static_cast<Stream *>(arg);
    if (play) {
        return play->subtitleThread();
    }
    return NEGATIVE(S_ERROR);
}

static int decodeInterruptCallback(void *ctx) {
    auto *is = static_cast<VideoState *>(ctx);
    return is->abortRequest;
}

Stream::Stream() = default;

Stream::~Stream() = default;

int Stream::create() {
    avformat_network_init();
    av_init_packet(&flushPacket);
    flushPacket.data = (uint8_t *) &flushPacket;
    return POSITIVE;
}

int Stream::destroy() {
    flushPacket.data = nullptr;
    av_packet_unref(&flushPacket);
    msgQueue = nullptr;
    avformat_network_deinit();
    return POSITIVE;
}

int Stream::stop() {
    // TODO
    return POSITIVE;
}

int Stream::shutdown() {
    streamsClose();
    return POSITIVE;
}

int Stream::waitStop() {
    // TODO
    return POSITIVE;
}

int Stream::prepareStream(const char *fileName) {
    ALOGD(STREAM_TAG, __func__);

    options->inputFileName = fileName != nullptr ? av_strdup(fileName) : nullptr;

    videoState = streamsOpen();

    if (!videoState) {
        ALOGE(STREAM_TAG, "%s Failed to initialize VideoState!", __func__);
        return NEGATIVE(S_NOT_CREATE_VIDEO_STATE);
    }

    return POSITIVE;
}

VideoState *Stream::streamsOpen() {
    ALOGD(STREAM_TAG, __func__);

    if (!options->inputFileName) {
        ALOGE(STREAM_TAG, "%s input file name is null", __func__);
        return nullptr;
    }

    auto *is = new VideoState();
    if (!is) {
        ALOGD(STREAM_TAG, "%s create video state oom", __func__);
        return nullptr;
    }

    if (is->videoFrameQueue.init(&is->videoPacketQueue, VIDEO_QUEUE_SIZE, 1) < 0) {
        ALOGE(STREAM_TAG, "%s video frame packetQueue init fail", __func__);
        streamsClose();
        return nullptr;
    }

    if (is->audioFrameQueue.init(&is->audioPacketQueue, AUDIO_QUEUE_SIZE, 1) < 0) {
        ALOGE(STREAM_TAG, "%s audio frame packetQueue init fail", __func__);
        streamsClose();
        return nullptr;
    }

    if (is->subtitleFrameQueue.init(&is->subtitlePacketQueue, SUBTITLE_QUEUE_SIZE, 0) < 0) {
        ALOGE(STREAM_TAG, "%s subtitle frame packetQueue init fail", __func__);
        streamsClose();
        return nullptr;
    }

    if (is->videoPacketQueue.init(&flushPacket) < 0 ||
        is->audioPacketQueue.init(&flushPacket) < 0 ||
        is->subtitlePacketQueue.init(&flushPacket) < 0) {
        ALOGE(STREAM_TAG, "%s packet packetQueue init fail", __func__);
        streamsClose();
        return nullptr;
    }

    if (!(is->continueReadThread = new Mutex())) {
        ALOGE(STREAM_TAG, "%s create continue read thread mutex fail", __func__);
        streamsClose();
        return nullptr;
    }

    if (is->videoClock.init(&is->videoPacketQueue.serial) < 0 ||
        is->audioClock.init(&is->audioPacketQueue.serial) < 0 ||
        is->exitClock.init(&is->subtitlePacketQueue.serial) < 0) {
        ALOGE(STREAM_TAG, "%s init clock fail", __func__);
        streamsClose();
        return nullptr;
    }

    is->fileName = av_strdup(options->inputFileName);
    is->audioClockSerial = -1;
    is->inputFormat = options->inputFormat;
    is->yTop = 0;
    is->xLeft = 0;
    is->audioVolume = getStartupVolume();
    is->audioMuted = 0;
    is->syncType = options->syncType;
    is->readThread = new Thread(innerReadThread, this, "Read   ");

    if (!is->readThread) {
        ALOGE(STREAM_TAG, "%s create read thread fail", __func__);
        streamsClose();
        return nullptr;
    }

    return is;
}

void Stream::streamsClose() {
    ALOGD(STREAM_TAG, __func__);

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

        delete videoState;
        videoState = nullptr;
    }
}

/// this thread gets the stream from the disk or the network
/// work thread
int Stream::readThread() {
    ALOGD(STREAM_TAG, __func__);

    if (!videoState) {
        return NEGATIVE(S_NULL);
    }

    AVFormatContext *formatContext = nullptr;
    AVPacket packet1;
    AVPacket *packet = &packet1;
    auto *waitMutex = new Mutex();
    AVDictionaryEntry *dictionaryEntry;
    int streamIndex[AVMEDIA_TYPE_NB];
    int scanAllPmtsSet = 0;
    int ret = 0;

    memset(streamIndex, -1, sizeof(streamIndex));

    videoState->videoLastStreamIndex = videoState->videoStreamIndex = -1;
    videoState->audioLastStreamIndex = videoState->audioStreamIndex = -1;
    videoState->subtitleLastStreamIndex = videoState->subtitleStreamIndex = -1;
    videoState->eof = 0;

    formatContext = avformat_alloc_context();
    if (!formatContext) {
        ALOGE(STREAM_TAG, "%s avformat could not allocate context", __func__);
        return NEGATIVE(S_NOT_MEMORY);
    }

    formatContext->interrupt_callback.callback = decodeInterruptCallback;
    formatContext->interrupt_callback.opaque = videoState;

    if (formatContext->duration) {
        options->duration = formatContext->duration;
    }

    if (options->inputFormatName) {
        videoState->inputFormat = av_find_input_format(options->inputFormatName);
    }

    // https://zhuanlan.zhihu.com/p/43672062
    // scan_all_pmts, 是mpegts的一个选项，这里在没有设定该选项的时候，强制设为1
    // scan_all_pmts, 扫描全部的ts流的"Program Map Table"表。
    // TODO: 针对MPEGTS的特殊处理
    if (!av_dict_get(options->format, SCAN_ALL_PMTS, nullptr, AV_DICT_MATCH_CASE)) {
        av_dict_set(&options->format, SCAN_ALL_PMTS, "1", AV_DICT_DONT_OVERWRITE);
        scanAllPmtsSet = 1;
    }

    if (avformat_open_input(&formatContext, videoState->fileName, videoState->inputFormat, &options->format) < 0) {
        ALOGE(STREAM_TAG, "%s avformat could not open input", __func__);
        closeReadThread(videoState, formatContext);
        return NEGATIVE(S_NOT_OPEN_INPUT);
    }

    // TODO: 还原MPEGTS的特殊处理标记
    if (scanAllPmtsSet) {
        av_dict_set(&options->format, SCAN_ALL_PMTS, nullptr, AV_DICT_MATCH_CASE);
    }

    if (msgQueue) {
        msgQueue->notifyMsg(Msg::MSG_OPEN_INPUT);
    }

    if ((dictionaryEntry = av_dict_get(options->format, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
        ALOGE(STREAM_TAG, "%s option %s not found.", __func__, dictionaryEntry->key);
        closeReadThread(videoState, formatContext);
        return NEGATIVE(S_NOT_FOUND_OPTION);
    }

    videoState->formatContext = formatContext;

    if (options->generateMissingPts) {
        formatContext->flags |= AVFMT_FLAG_GENPTS;
    }

    av_format_inject_global_side_data(formatContext);

    if (options->findStreamInfoFillMissingInformation) {
        AVDictionary **opts = setupFindStreamInfoOpts(formatContext, options->codec);
        int ret = avformat_find_stream_info(formatContext, opts);
        for (int i = 0; i < formatContext->nb_streams; i++) {
            av_dict_free(&opts[i]);
        }
        av_freep(&opts);
        if (ret < 0) {
            ALOGD(STREAM_TAG, "%s %s: could not find codec parameters", __func__, videoState->fileName);
            closeReadThread(videoState, formatContext);
            return NEGATIVE(S_NOT_FIND_STREAM_INFO);
        }
        if (msgQueue) {
            msgQueue->notifyMsg(Msg::MSG_FIND_STREAM_INFO);
        }
    }

    // I/O context.
    if (formatContext->pb) {
        formatContext->pb->eof_reached = 0;
    }

    if (options->seekByBytes < 0) {
        // AVFMT_NO_BYTE_SEEK
        // Format allows timestamp discontinuities
        options->seekByBytes = formatContext->iformat->flags & AVFMT_TS_DISCONT && strcmp(FORMAT_OGG, formatContext->iformat->name) != 0;
    }

    videoState->maxFrameDuration = (formatContext->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;

    // get window title from metadata
    if (!options->videoTitle && (dictionaryEntry = av_dict_get(formatContext->metadata, TITLE, nullptr, 0))) {
        options->videoTitle = av_asprintf("%s - %s", dictionaryEntry->value, options->inputFileName);
    }

    /* if seeking requested, we execute it */
    if (options->startTime != AV_NOPTS_VALUE) {
        int64_t timestamp = options->startTime;
        /* add the stream start time */
        if (formatContext->start_time != AV_NOPTS_VALUE) {
            timestamp += formatContext->start_time;
        }
        if (avformat_seek_file(formatContext, -1, INT64_MIN, timestamp, INT64_MAX, 0) < 0) {
            ALOGE(STREAM_TAG, "%s %s: could not seek to position %0.3f", __func__, videoState->fileName, (double) timestamp / AV_TIME_BASE);
        }
    }

    videoState->realTime = isRealTime(formatContext);

    if (options->infiniteBuffer < 0 && videoState->realTime) {
        options->infiniteBuffer = 1;
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
            ALOGD(STREAM_TAG, "%s Stream specifier %s does not match any stream", __func__, options->wantedStreamSpec[i]);
            streamIndex[i] = INT_MAX;
        }
    }

    if (options && !options->videoDisable) {
        streamIndex[AVMEDIA_TYPE_VIDEO] = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, streamIndex[AVMEDIA_TYPE_VIDEO], -1, nullptr, 0);
    }

    if (options && !options->audioDisable) {
        streamIndex[AVMEDIA_TYPE_AUDIO] = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, streamIndex[AVMEDIA_TYPE_AUDIO], streamIndex[AVMEDIA_TYPE_VIDEO], nullptr, 0);
    }

    if (options && !options->videoDisable && !options->subtitleDisable) {
        streamIndex[AVMEDIA_TYPE_SUBTITLE] = av_find_best_stream(formatContext, AVMEDIA_TYPE_SUBTITLE, streamIndex[AVMEDIA_TYPE_SUBTITLE], (streamIndex[AVMEDIA_TYPE_AUDIO] >= 0 ? streamIndex[AVMEDIA_TYPE_AUDIO] : streamIndex[AVMEDIA_TYPE_VIDEO]), nullptr, 0);
    }

    videoState->showMode = options->showMode;

    if (streamIndex[AVMEDIA_TYPE_VIDEO] >= 0) {
        AVStream *stream = formatContext->streams[streamIndex[AVMEDIA_TYPE_VIDEO]];
        AVCodecParameters *codecParameters = stream->codecpar;
        AVRational sampleAspectRatio = av_guess_sample_aspect_ratio(formatContext, stream, nullptr);
        if (surface && codecParameters->width) {
            surface->setVideoSize(codecParameters->width, codecParameters->height, sampleAspectRatio);
        }
    }

    if (options && options->showStatus) {
        av_dump_format(formatContext, 0, videoState->fileName, 0);
    }

    if (options) {
        options->showOptions();
    }

    if (options->videoDisable && options->audioDisable) {
        ALOGD(STREAM_TAG, "%s disable all stream", __func__);
        msgQueue->notifyMsg(Msg::MSG_DISABLE_ALL_STREAM);
        msgQueue->notifyMsg(Msg::REQ_QUIT);
        return NEGATIVE(S_DISABLE_ALL_STREAM);
    }

    if (streamIndex[AVMEDIA_TYPE_AUDIO] >= 0) {
        if (streamComponentOpen(streamIndex[AVMEDIA_TYPE_AUDIO])) {
            if (msgQueue) {
                msgQueue->notifyMsg(Msg::MSG_AUDIO_COMPONENT_OPEN);
            }
        }
    } else {
        videoState->syncType = options->syncType = SYNC_TYPE_VIDEO_MASTER;
    }

    ret = -1;
    if (streamIndex[AVMEDIA_TYPE_VIDEO] >= 0) {
        ret = streamComponentOpen(streamIndex[AVMEDIA_TYPE_VIDEO]);
        if (ret) {
            if (msgQueue) {
                msgQueue->notifyMsg(Msg::MSG_VIDEO_COMPONENT_OPEN);
            }
        }
    }

    if (videoState->showMode == SHOW_MODE_NONE) {
        videoState->showMode = ret >= 0 ? SHOW_MODE_VIDEO : SHOW_MODE_RDFT;
    }

    if (streamIndex[AVMEDIA_TYPE_SUBTITLE] >= 0) {
        if (streamComponentOpen(streamIndex[AVMEDIA_TYPE_SUBTITLE])) {
            if (msgQueue) {
                msgQueue->notifyMsg(Msg::MSG_SUBTITLE_COMPONENT_OPEN);
            }
        }
    }

    if (msgQueue) {
        msgQueue->notifyMsg(Msg::MSG_COMPONENTS_OPEN);
    }

    if (videoState->videoStreamIndex < 0 && videoState->audioStreamIndex < 0) {
        ALOGD(STREAM_TAG, "%s failed to open file '%s' or configure filter graph", __func__, videoState->fileName);
        closeReadThread(videoState, formatContext);
        return NEGATIVE(S_NOT_OPEN_FILE);
    }

    if (videoState->videoStream && videoState->videoStream->codecpar) {
        AVCodecParameters *codecParameters = videoState->videoStream->codecpar;
        if (msgQueue) {
            msgQueue->notifyMsg(Msg::MSG_VIDEO_SIZE_CHANGED, codecParameters->width, codecParameters->height);
            msgQueue->notifyMsg(Msg::MSG_SAR_CHANGED, codecParameters->width, codecParameters->height);
        }
    }

    if (msgQueue) {
        msgQueue->notifyMsg(Msg::MSG_PREPARED);
    }

    if (msgQueue) {
        msgQueue->notifyMsg(Msg::REQ_START);
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

#if CONFIG_RTSP_DEMUXER || CONFIG_MMSH_PROTOCOL
        if (videoState->paused && (!strcmp(formatContext->iformat->name, "rtsp") || (formatContext->pb && !strncmp(options->inputFileName, "mmsh:", 5)))) {
            /* wait 10 ms to avoid trying to get another packet */
            /* XXX: horrible */
            waitMutex->mutexLock();
            videoState->continueReadThread->condWaitTimeout(waitMutex, 10);
            waitMutex->mutexUnLock();
            continue;
        }
#endif

        if (videoState->seekReq) {
            int64_t seekTarget = videoState->seekPos;
            int64_t seekMin = videoState->seekRel > 0 ? (seekTarget - videoState->seekRel + 2) : INT64_MIN;
            int64_t seekMax = videoState->seekRel < 0 ? (seekTarget - videoState->seekRel - 2) : INT64_MAX;
            if (avformat_seek_file(videoState->formatContext, -1, seekMin, seekTarget, seekMax, videoState->seekFlags) < 0) {
                ALOGD(STREAM_TAG, "%s %s: error while seeking", __func__, videoState->formatContext->url);
            } else {
                if (videoState->audioStreamIndex >= 0) {
                    videoState->audioPacketQueue.flush();
                    videoState->audioPacketQueue.put(&flushPacket);
                }
                if (videoState->videoStreamIndex >= 0) {
                    videoState->videoPacketQueue.flush();
                    videoState->videoPacketQueue.put(&flushPacket);
                }
                if (videoState->subtitleStreamIndex >= 0) {
                    videoState->subtitlePacketQueue.flush();
                    videoState->subtitlePacketQueue.put(&flushPacket);
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

        if (videoState->queueAttachmentsReq) {
            // https://segmentfault.com/a/1190000018373504?utm_source=tag-newest
            // 它和mp3文件有关，是一个流的标志
            if (videoState->videoStream && videoState->videoStream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
                AVPacket copy = {nullptr};
                if ((av_packet_ref(&copy, &videoState->videoStream->attached_pic)) < 0) {
                    ALOGD(STREAM_TAG, "%s av_packet_ref failed", __func__);
                    closeReadThread(videoState, formatContext);
                    return NEGATIVE(S_NOT_ATTACHED_PIC);
                }
                videoState->videoPacketQueue.put(&copy);
                videoState->videoPacketQueue.putNullPacket(videoState->videoStreamIndex);
            }
            videoState->queueAttachmentsReq = 0;
        }

        // if the queue are full, no need to read more
        if (isNoReadMore()) {
            waitMutex->mutexLock();
            /* wait 10 ms */
            videoState->continueReadThread->condWaitTimeout(waitMutex, 10);
            waitMutex->mutexUnLock();
            continue;
        }

        if (isRetryPlay()) {
            bool isShouldLoop = options->loopTimes != 1 && (!options->loopTimes || --options->loopTimes);
            if (isShouldLoop) {
                // Seek To Start Position
                streamSeek(options->startTime != AV_NOPTS_VALUE ? options->startTime : 0, 0, 0);
            } else if (options->autoExit) {
                closeReadThread(videoState, formatContext);
                msgQueue->notifyMsg(Msg::REQ_QUIT);
                ALOGD(STREAM_TAG, "%s auto exit", __func__);
                return NEGATIVE(S_EOF);
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
                ALOGD(STREAM_TAG, "%s I/O context error ", __func__);
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

        int packetInPlayRange = isPacketInPlayRange(formatContext, packet);
        if (packet->stream_index == videoState->audioStreamIndex && packetInPlayRange) {
            videoState->audioPacketQueue.put(packet);
        } else if (packet->stream_index == videoState->videoStreamIndex && packetInPlayRange && !(videoState->videoStream->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
            videoState->videoPacketQueue.put(packet);
        } else if (packet->stream_index == videoState->subtitleStreamIndex && packetInPlayRange) {
            videoState->subtitlePacketQueue.put(packet);
        } else {
            av_packet_unref(packet);
        }

    } // for end
    return POSITIVE;
}

int Stream::videoThread() {
    ALOGD(STREAM_TAG, __func__);

    VideoState *is = videoState;
    AVFrame *frame = av_frame_alloc();
    AVRational timeBase = is->videoStream->time_base;
    AVRational frameRate = av_guess_frame_rate(is->formatContext, is->videoStream, nullptr);
    double pts;
    double duration;
    int ret;

#if CONFIG_AVFILTER
    AVFilterGraph *filterGraph = nullptr;
    AVFilterContext *filterOut = nullptr, *filterIn = nullptr;
    int lastWidth = 0;
    int lastHeight = 0;
    // AVPixelFormat
    int lastFormat = -2;
    int lastPacketSerial = -1;
    int lastFilterIndex = 0;
#endif

    if (!frame) {
        return NEGATIVE(S_NOT_MEMORY);
    }

    for (;;) {

        ret = getVideoFrame(frame);

        if (ret == NEGATIVE(S_FRAME_DROP)) {
            continue;
        }

        if (IS_NEGATIVE(ret)) {
            av_frame_free(&frame);
            ALOGE(STREAM_TAG, "%s not get video frame ret = %d ", __func__, ret);
            return NEGATIVE(S_NOT_GET_VIDEO_FRAME);
        }

#if CONFIG_AVFILTER

        bool isNoSameWidth = lastWidth != frame->width;
        bool isNoSameHeight = lastHeight != frame->height;
        bool isNoSameFormat = lastFormat != frame->format;
        bool isNoSamePacketSerial = lastPacketSerial != is->videoDecoder.packetSerial;
        bool isNoSameVFilterIdx = lastFilterIndex != is->filterIndex;
        bool isNeedConfigureFilter = isNoSameWidth || isNoSameHeight || isNoSameFormat || isNoSamePacketSerial || isNoSameVFilterIdx;

        // https://ffmpeg.org/ffmpeg-filters.html
        if (isNeedConfigureFilter) {

            ALOGD(STREAM_TAG, "%s Video frame changed from size:%dx%d format:%s serial:%d to size:%dx%d format:%s serial:%d", __func__,
                  lastWidth, lastHeight,
                  (const char *) av_x_if_null(av_get_pix_fmt_name((AVPixelFormat) lastFormat), "none"), lastPacketSerial,
                  frame->width, frame->height,
                  (const char *) av_x_if_null(av_get_pix_fmt_name((AVPixelFormat) frame->format), "none"), is->videoDecoder.packetSerial);

            avfilter_graph_free(&filterGraph);

            filterGraph = avfilter_graph_alloc();

            if (!filterGraph) {
                avfilter_graph_free(&filterGraph);
                av_frame_free(&frame);
                return NEGATIVE(ENOMEM);
            }

            filterGraph->nb_threads = options->filterNumberThreads;

            if ((ret = configureVideoFilters(filterGraph, is, options->filtersList ? options->filtersList[is->filterIndex] : nullptr, frame)) < 0) {
                msgQueue->notifyMsg(Msg::REQ_QUIT);
                avfilter_graph_free(&filterGraph);
                av_frame_free(&frame);
                return NEGATIVE(S_NOT_CONFIGURE_VIDEO_FILTERS);
            }

            filterIn = is->videoInFilter;
            filterOut = is->videoOutFilter;
            lastWidth = frame->width;
            lastHeight = frame->height;
            lastFormat = frame->format;
            lastPacketSerial = is->videoDecoder.packetSerial;
            lastFilterIndex = is->filterIndex;
            frameRate = av_buffersink_get_frame_rate(filterOut);
        }

        ret = av_buffersrc_add_frame(filterIn, frame);

        if (ret < 0) {
            avfilter_graph_free(&filterGraph);
            av_frame_free(&frame);
            return NEGATIVE(S_NOT_ADD_FRAME_TO_FILTER);
        }

        while (ret >= 0) {
            is->frameSinkFilterStartTime = av_gettime_relative() / 1000000.0;
            // 没有输入端的滤镜称为“源(source)”，没有输出端的滤镜称为“槽(sink)”
            // https://blog.csdn.net/joee33/article/details/51946712
            ret = av_buffersink_get_frame_flags(filterOut, frame, 0);
            if (ret < 0) {
                if (ret == AVERROR_EOF) {
                    is->videoDecoder.finished = is->videoDecoder.packetSerial;
                }
                ret = 0;
                break;
            }
            is->frameSinkFilterConsumeTime = av_gettime_relative() / 1000000.0 - is->frameSinkFilterStartTime;
            if (fabs(is->frameSinkFilterConsumeTime) > NO_SYNC_THRESHOLD / 10.0) {
                is->frameSinkFilterConsumeTime = 0;
            }
            timeBase = av_buffersink_get_time_base(filterOut);
#endif

            duration = (frameRate.num && frameRate.den ? av_q2d((AVRational) {frameRate.den, frameRate.num}) : 0);
            pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(timeBase);
            ret = queueFrameToFrameQueue(frame, pts, duration, frame->pkt_pos, is->videoDecoder.packetSerial);
            av_frame_unref(frame);

#if CONFIG_AVFILTER
            if (is->videoPacketQueue.serial != is->videoDecoder.packetSerial) {
                break;
            }
        }
#endif

        if (IS_NEGATIVE(ret)) {
            av_frame_free(&frame);
            ALOGE(STREAM_TAG, "%s not queue picture", __func__);
            return NEGATIVE(S_NOT_QUEUE_PICTURE);
        }
    }

    ALOGD(STREAM_TAG, "%s end", __func__);
    return POSITIVE;
}

int Stream::subtitleThread() {
    ALOGD(STREAM_TAG, __func__);
    Frame *frame;
    int gotSubtitle;
    double pts;
    for (;;) {
        if (!(frame = videoState->subtitleFrameQueue.peekWritable())) {
            return NEGATIVE(S_NOT_FRAME_WRITEABLE);
        }
        if ((gotSubtitle = decoderDecodeFrame(&videoState->subtitleDecoder, nullptr, &frame->sub)) < 0) {
            break;
        }
        pts = 0;
        if (gotSubtitle && frame->sub.format == 0) {
            if (frame->sub.pts != AV_NOPTS_VALUE) {
                pts = frame->sub.pts / (double) AV_TIME_BASE;
            }
            frame->pts = pts;
            frame->serial = videoState->subtitleDecoder.packetSerial;
            frame->width = videoState->subtitleDecoder.codecContext->width;
            frame->height = videoState->subtitleDecoder.codecContext->height;
            frame->uploaded = 0;
            /* now we can update the picture count */
            videoState->subtitleFrameQueue.push();
        } else if (gotSubtitle) {
            avsubtitle_free(&frame->sub);
        }
    }
    return POSITIVE;
}

int Stream::cmpAudioFormats(AVSampleFormat fmt1, int64_t channel_count1, AVSampleFormat fmt2, int64_t channel_count2) {
    /* If channel count == 1, planar and non-planar formats are the same */
    if (channel_count1 == 1 && channel_count2 == 1) {
        return av_get_packed_sample_fmt(fmt1) != av_get_packed_sample_fmt(fmt2);
    } else {
        return channel_count1 != channel_count2 || fmt1 != fmt2;
    }
}

int Stream::audioThread() {
    ALOGD(STREAM_TAG, __func__);

    VideoState *is = videoState;
    AVFrame *avFrame = av_frame_alloc();
    Frame *frame;
    AVRational timeBase;
    int gotFrame = 0;
    int ret = 0;

#if CONFIG_AVFILTER
    int lastSerial = -1;
    int64_t decChannelLayout;
    int reconfigure;
#endif

    if (!avFrame) {
        return NEGATIVE(S_NOT_MEMORY);
    }

    do {
        if ((gotFrame = decoderDecodeFrame(&is->audioDecoder, avFrame, nullptr)) < 0) {
            ALOGE(STREAM_TAG, "%s audio decoderDecodeFrame failure ret = %d", __func__, gotFrame);
            goto the_end;
        }

        if (gotFrame) {
            timeBase = (AVRational) {1, avFrame->sample_rate};

#if CONFIG_AVFILTER
            decChannelLayout = getValidChannelLayout(avFrame->channel_layout, avFrame->channels);

            reconfigure = cmpAudioFormats(is->audioFilterSrc.fmt, is->audioFilterSrc.channels,
                                          (AVSampleFormat) avFrame->format, avFrame->channels) ||
                          is->audioFilterSrc.channel_layout != decChannelLayout ||
                          is->audioFilterSrc.freq != avFrame->sample_rate ||
                          is->audioDecoder.packetSerial != lastSerial;

            if (reconfigure) {
                char buf1[1024], buf2[1024];
                av_get_channel_layout_string(buf1, sizeof(buf1), -1, is->audioFilterSrc.channel_layout);
                av_get_channel_layout_string(buf2, sizeof(buf2), -1, decChannelLayout);

                ALOGD(STREAM_TAG, "%s Audio avFrame changed from rate:%d ch:%d fmt:%s layout:%s serial:%d to rate:%d ch:%d fmt:%s layout:%s serial:%d", __func__,
                      is->audioFilterSrc.freq, is->audioFilterSrc.channels, av_get_sample_fmt_name(is->audioFilterSrc.fmt), buf1, lastSerial,
                      avFrame->sample_rate, avFrame->channels, av_get_sample_fmt_name((AVSampleFormat) avFrame->format), buf2, is->audioDecoder.packetSerial);

                is->audioFilterSrc.fmt = (AVSampleFormat) avFrame->format;
                is->audioFilterSrc.channels = avFrame->channels;
                is->audioFilterSrc.channel_layout = decChannelLayout;
                is->audioFilterSrc.freq = avFrame->sample_rate;
                lastSerial = is->audioDecoder.packetSerial;

                if ((ret = configureAudioFilters(options->afilters, 1)) < 0) {
                    ALOGE(STREAM_TAG, "%s configure audio filter failure ret = %d", __func__, ret);
                    goto the_end;
                }
            }

            if ((ret = av_buffersrc_add_frame(is->inAudioFilter, avFrame)) < 0) {
                ALOGE(STREAM_TAG, "%s add src frame failure ret = %d", __func__, ret);
                goto the_end;
            }

            while ((ret = av_buffersink_get_frame_flags(is->outAudioFilter, avFrame, 0)) >= 0) {
                timeBase = av_buffersink_get_time_base(is->outAudioFilter);
#endif
                if (!(frame = videoState->audioFrameQueue.peekWritable())) {
                    ALOGE(STREAM_TAG, "%s not queue audio", __func__);
                    goto the_end;
                }

                frame->pts = (avFrame->pts == AV_NOPTS_VALUE) ? NAN : avFrame->pts * av_q2d(timeBase);
                frame->pos = avFrame->pkt_pos;
                frame->serial = videoState->audioDecoder.packetSerial;
                frame->duration = av_q2d((AVRational) {avFrame->nb_samples, avFrame->sample_rate});

                av_frame_move_ref(frame->frame, avFrame);
                videoState->audioFrameQueue.push();

#if CONFIG_AVFILTER
                if (is->audioPacketQueue.serial != is->audioDecoder.packetSerial) {
                    break;
                }
            }
            if (ret == AVERROR_EOF) {
                is->audioDecoder.finished = is->audioDecoder.packetSerial;
            }
#endif
        }
    } while (ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF);

    the_end:
#if CONFIG_AVFILTER
    avfilter_graph_free(&is->agraph);
#endif
    av_frame_free(&avFrame);
    return ret;
}

int Stream::isPacketInPlayRange(const AVFormatContext *formatContext, const AVPacket *packet) const {

    if (options->duration == AV_NOPTS_VALUE) {
        return POSITIVE;
    }

    /* check if packet videoState in stream range specified by user, then packetQueue, otherwise discard */
    int64_t streamStartTime = formatContext->streams[packet->stream_index]->start_time;

    // https://baike.baidu.com/item/PTS/13977433
    int64_t packetTimestamp = (packet->pts == AV_NOPTS_VALUE) ? packet->dts : packet->pts;
    int64_t diffTimestamp = packetTimestamp - (streamStartTime != AV_NOPTS_VALUE ? streamStartTime : 0);

    double diffTime = diffTimestamp * av_q2d(formatContext->streams[packet->stream_index]->time_base);
    double startTime = (double) (options->startTime != AV_NOPTS_VALUE ? options->startTime : 0) / AV_TIME_BASE;
    double duration = (double) options->duration / AV_TIME_BASE;

    return (options->duration == AV_NOPTS_VALUE) || ((diffTime - startTime) <= duration);
}

bool Stream::isRetryPlay() const {
    // 未暂停
    bool isNoPause = !videoState->paused;
    // 未初始化音频流 或者 解码结束 同时 无可用帧
    bool isNoUseAudioFrame = !videoState->audioStream || (videoState->audioDecoder.finished == videoState->audioPacketQueue.serial && videoState->audioFrameQueue.numberRemaining() == 0);
    // 未初始化视频流 或者 解码结束 同时 无可用帧
    bool isNoUseVideoFrame = !videoState->videoStream || (videoState->videoDecoder.finished == videoState->videoPacketQueue.serial && videoState->videoFrameQueue.numberRemaining() == 0);
    return isNoPause && isNoUseAudioFrame && isNoUseVideoFrame;
}

bool Stream::isNoReadMore() {
    /* if the packetQueue are full, no need to read more */
    bool isNoInfiniteBuffer = options->infiniteBuffer < 1;
    bool isNoEnoughMemory = (videoState->audioPacketQueue.memorySize + videoState->videoPacketQueue.memorySize + videoState->subtitlePacketQueue.memorySize) > MAX_QUEUE_SIZE;
    bool isVideoEnoughPackets = streamHasEnoughPackets(videoState->videoStream, videoState->videoStreamIndex, &videoState->videoPacketQueue);
    bool isAudioEnoughPackets = streamHasEnoughPackets(videoState->audioStream, videoState->audioStreamIndex, &videoState->audioPacketQueue);
    bool isSubtitleEnoughPackets = streamHasEnoughPackets(videoState->subtitleStream, videoState->subtitleStreamIndex, &videoState->subtitlePacketQueue);
    return isNoInfiniteBuffer && (isNoEnoughMemory || (isAudioEnoughPackets && isVideoEnoughPackets && isSubtitleEnoughPackets));
}

void Stream::closeReadThread(const VideoState *is, AVFormatContext *formatContext) const {
    ALOGD(STREAM_TAG, __func__);
    if (formatContext && !is->formatContext) {
        avformat_close_input(&formatContext);
        avformat_free_context(formatContext);
    }
}

int Stream::isRealTime(AVFormatContext *formatContext) {
    if (!strcmp(formatContext->iformat->name, FORMAT_RTP) || !strcmp(formatContext->iformat->name, FORMAT_RTSP) || !strcmp(formatContext->iformat->name, FORMAT_SDP)) {
        return POSITIVE;
    }
    if (formatContext->pb && (!strncmp(formatContext->url, URL_FORMAT_RTP, 4) || !strncmp(formatContext->url, URL_FORMAT_UDP, 4))) {
        return POSITIVE;
    }
    return NEGATIVE(S_ERROR);
}

int Stream::streamComponentOpen(int streamIndex) {

    ALOGD(STREAM_TAG, "%s streamIndex=%d", __func__, streamIndex);

    AVFormatContext *formatContext = videoState->formatContext;
    AVCodecContext *codecContext;
    AVCodec *codec;
    AVDictionary *opts = nullptr;
    AVDictionaryEntry *t = nullptr;
    int sampleRate;
    int nbChannels;
    int64_t channelLayout;
    int streamLowResolution = options->lowResolution;
    int ret = 0;

    if (streamIndex < 0 || streamIndex >= formatContext->nb_streams) {
        return NEGATIVE(S_NOT_VALID_STREAM_INDEX);
    }

    codecContext = avcodec_alloc_context3(nullptr);
    if (!codecContext) {
        return NEGATIVE(S_NOT_ALLOC_CODEC_CONTEXT);
    }

    if (avcodec_parameters_to_context(codecContext, formatContext->streams[streamIndex]->codecpar) < 0) {
        avcodec_free_context(&codecContext);
        return NEGATIVE(S_NOT_CODEC_PARAMS_CONTEXT);
    }

    codecContext->pkt_timebase = formatContext->streams[streamIndex]->time_base;

    codec = avcodec_find_decoder(codecContext->codec_id);

    const char *forcedCodecName = getForcedCodecName(streamIndex, codecContext);
    if (forcedCodecName) {
        codec = avcodec_find_decoder_by_name(forcedCodecName);
    }
    if (!codec) {
        if (forcedCodecName) {
            ALOGD(STREAM_TAG, "%s No codec could be found with name '%s'", __func__, forcedCodecName);
        } else {
            ALOGD(STREAM_TAG, "%sNo decoder could be found for codec %s", __func__, avcodec_get_name(codecContext->codec_id));
        }
        avcodec_free_context(&codecContext);
        return NEGATIVE(S_NOT_FOUND_CODER);
    }

    codecContext->codec_id = codec->id;

    if (streamLowResolution > codec->max_lowres) {
        ALOGD(STREAM_TAG, "%s The maximum value for low Resolution supported by the decoder is %d", __func__, codec->max_lowres);
        streamLowResolution = codec->max_lowres;
    }
    codecContext->lowres = streamLowResolution;

    if (options->fast) {
        codecContext->flags2 |= AV_CODEC_FLAG2_FAST;
    }

    opts = filter_codec_opts(options->codec, codecContext->codec_id, formatContext, formatContext->streams[streamIndex], codec);

    if (!av_dict_get(opts, KEY_THREADS, nullptr, 0)) {
        av_dict_set(&opts, KEY_THREADS, "auto", 0);
    }

    if (streamLowResolution) {
        av_dict_set_int(&opts, KEY_LOW_RESOLUTION, streamLowResolution, 0);
    }

    if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO || codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
        av_dict_set(&opts, KEY_REF_COUNTED_FRAMES, "1", 0);
    }

    if (avcodec_open2(codecContext, codec, &opts) < 0) {
        avcodec_free_context(&codecContext);
        return NEGATIVE(S_NOT_OPEN_DECODE);
    }

    if ((t = av_dict_get(opts, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
        ALOGE(STREAM_TAG, "%s Option %s not found.", __func__, t->key);
        return NEGATIVE(S_NOT_FOUND_OPTION);
    }

    videoState->eof = 0;

    // discard useless packets like 0 size packets in avi
    formatContext->streams[streamIndex]->discard = AVDISCARD_DEFAULT;

    switch (codecContext->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
#if CONFIG_AVFILTER
            AVFilterContext *sink;
            videoState->audioFilterSrc.freq = codecContext->sample_rate;
            videoState->audioFilterSrc.channels = codecContext->channels;
            videoState->audioFilterSrc.channel_layout = getValidChannelLayout(codecContext->channel_layout, codecContext->channels);
            videoState->audioFilterSrc.fmt = codecContext->sample_fmt;
            if (configureAudioFilters(options->afilters, 0) < 0) {
                av_dict_free(&opts);
                return NEGATIVE(S_NOT_CONFIGURE_AUDIO_FILTERS);
            }
            sink = videoState->outAudioFilter;
            sampleRate = av_buffersink_get_sample_rate(sink);
            nbChannels = av_buffersink_get_channels(sink);
            channelLayout = av_buffersink_get_channel_layout(sink);
#else
        sampleRate = codecContext->sample_rate;
        nbChannels = codecContext->channels;
        channelLayout = codecContext->channel_layout;
#endif

            if (audio && (ret = audio->openAudio(channelLayout, nbChannels, sampleRate, &videoState->audioTarget)) < 0) {
                av_dict_free(&opts);
                return NEGATIVE(S_NOT_OPEN_AUDIO);
            }

            videoState->audioHwBufSize = ret;
            videoState->audioSrc = videoState->audioTarget;
            videoState->audioBufSize = 0;
            videoState->audioBufIndex = 0;

            /* init averaging filter */
            videoState->audioDiffAvgCoef = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
            videoState->audioDiffAvgCount = 0;
            /* since we do not have a precise anough audio FIFO fullness,we correct audio sync only if larger than this threshold */
            videoState->audioDiffThreshold = (double) (videoState->audioHwBufSize) / videoState->audioTarget.bytes_per_sec;

            videoState->audioStreamIndex = streamIndex;
            videoState->audioStream = formatContext->streams[streamIndex];
            videoState->audioDecoder.decoderInit(codecContext, &videoState->audioPacketQueue, videoState->continueReadThread);
            if ((videoState->formatContext->iformat->flags & (AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK)) && !videoState->formatContext->iformat->read_seek) {
                videoState->audioDecoder.startPts = videoState->audioStream->start_time;
                videoState->audioDecoder.startPtsTb = videoState->audioStream->time_base;
            }
            if (!videoState->audioDecoder.decoderStart("ADecode", innerAudioThread, this)) {
                av_dict_free(&opts);
                return NEGATIVE(S_NOT_AUDIO_DECODE_START);
            }
            if (audio) {
                audio->pauseAudio();
            }
            break;
        case AVMEDIA_TYPE_VIDEO:
            videoState->videoStreamIndex = streamIndex;
            videoState->videoStream = formatContext->streams[streamIndex];
            videoState->videoDecoder.decoderInit(codecContext, &videoState->videoPacketQueue, videoState->continueReadThread);
            if (!videoState->videoDecoder.decoderStart("VDecode", innerVideoThread, this)) {
                av_dict_free(&opts);
                return NEGATIVE(S_NOT_VIDEO_DECODE_START);
            }
            videoState->queueAttachmentsReq = 1;
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            videoState->subtitleStreamIndex = streamIndex;
            videoState->subtitleStream = formatContext->streams[streamIndex];
            videoState->subtitleDecoder.decoderInit(codecContext, &videoState->subtitlePacketQueue, videoState->continueReadThread);
            if (!videoState->subtitleDecoder.decoderStart("SDecode", innerSubtitleThread, this)) {
                av_dict_free(&opts);
                return NEGATIVE(S_NOT_SUBTITLE_DECODE_START);
            }
            break;
        default:
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
    return POSITIVE;
}

const char *Stream::getForcedCodecName(int streamIndex, const AVCodecContext *codecContext) const {
    const char *forcedCodecName = nullptr;
    switch (codecContext->codec_type) {
        case AVMEDIA_TYPE_AUDIO   :
            videoState->audioLastStreamIndex = streamIndex;
            forcedCodecName = options->forceAudioCodecName;
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            videoState->subtitleLastStreamIndex = streamIndex;
            forcedCodecName = options->forceSubtitleCodecName;
            break;
        case AVMEDIA_TYPE_VIDEO   :
            videoState->videoLastStreamIndex = streamIndex;
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
    return forcedCodecName;
}

bool Stream::streamHasEnoughPackets(AVStream *stream, int streamIndex, PacketQueue *packetQueue) {
    return streamIndex < 0 || packetQueue->abortRequest || (stream->disposition & AV_DISPOSITION_ATTACHED_PIC) || (packetQueue->packetSize > MIN_FRAMES && (!packetQueue->duration || av_q2d(stream->time_base) * packetQueue->duration > 1.0));
}

int Stream::streamComponentClose(AVStream *stream, int streamIndex) {
    AVFormatContext *ic = videoState->formatContext;
    AVCodecParameters *codecParameters;

    if (streamIndex < 0 || streamIndex >= ic->nb_streams) {
        return NEGATIVE(S_NOT_VALID_STREAM_INDEX);
    }

    codecParameters = ic->streams[streamIndex]->codecpar;

    switch (codecParameters->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            videoState->audioDecoder.decoderAbort(&videoState->audioFrameQueue);
            videoState->audioDecoder.decoderDestroy();
            swr_free(&videoState->audioSwrContext);
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
double Stream::getMasterClock() {
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

int Stream::getMasterSyncType() {
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

void Stream::streamSeek(int64_t pos, int64_t rel, int seekByBytes) {
    /* seek in the stream */
    if (!videoState->seekReq) {
        videoState->seekPos = pos;
        videoState->seekRel = rel;
        videoState->seekFlags &= ~AVSEEK_FLAG_BYTE;
        if (seekByBytes) {
            videoState->seekFlags |= AVSEEK_FLAG_BYTE;
        }
        videoState->seekReq = 1;
        videoState->continueReadThread->condSignal();
    }
}

int Stream::getVideoFrame(AVFrame *frame) {
    ALOGD(STREAM_TAG, __func__);

    int gotPicture = NEGATIVE(S_FRAME_DROP);

    if ((gotPicture = decoderDecodeFrame(&videoState->videoDecoder, frame, nullptr)) < 0) {
        ALOGE(STREAM_TAG, "%s video decoderDecodeFrame failure ret = %d", __func__, gotPicture);
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

                // diff > 0 当前帧显示时间略超于出主时钟
                // diff < 0 当前帧显示时间慢于主时钟
                double diff = dpts - getMasterClock();

                bool isNoNan = !isnan(diff);
                bool isNoSync = fabs(diff) < NO_SYNC_THRESHOLD;
                bool isNeedCorrection = diff - videoState->frameSinkFilterConsumeTime < 0;
                bool isSameSerial = videoState->videoDecoder.packetSerial == videoState->videoClock.serial;
                bool isLegalSize = videoState->videoPacketQueue.packetSize > 0;

                if (isNoNan && isNoSync && isNeedCorrection && isSameSerial && isLegalSize) {
                    // 显示过慢，丢掉当前帧
                    videoState->frameDropsEarly++;
                    av_frame_unref(frame);
                    gotPicture = NEGATIVE(S_FRAME_DROP);
                }
            }
        }
    }

    return gotPicture;
}

int Stream::decoderDecodeFrame(Decoder *decoder, AVFrame *frame, AVSubtitle *subtitle) {


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
                            // 是否使用通过解码器估算过的时间
                            if (options->decoderReorderPts == -1) {
                                // frame timestamp estimated using various heuristics, in stream time base
                                frame->pts = frame->best_effort_timestamp;
                            } else if (!options->decoderReorderPts) {
                                // This is also the Presentation time of this AVFrame calculated from
                                // only AVPacket.dts values without pts values.
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
                    return NEGATIVE(S_EOF);
                }
                if (ret >= 0) {
                    return POSITIVE;
                }
            } while (ret != AVERROR(EAGAIN));
        }

        do {
            // 同步读取序列

            if (decoder->packetQueue->packetSize == 0) {
                decoder->emptyQueueCond->condSignal();
            }
            if (decoder->packetPending) {
                av_packet_move_ref(&packet, &decoder->packet);
                decoder->packetPending = 0;
            } else {
                if (IS_NEGATIVE(decoder->packetQueue->get(&packet, 1, &decoder->packetSerial))) {
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
                    ret = NEGATIVE(S_NOT_DECODE_SUBTITLE_FRAME);
                } else {
                    if (gotFrame && !packet.data) {
                        decoder->packetPending = 1;
                        av_packet_move_ref(&decoder->packet, &packet);
                    }
                    ret = gotFrame ? POSITIVE : (packet.data ? NEGATIVE(EAGAIN) : NEGATIVE(S_EOF));
                }
            } else {
                if (avcodec_send_packet(decoder->codecContext, &packet) == AVERROR(EAGAIN)) {
                    ALOGE(STREAM_TAG, "%s Receive_frame and send_packet both returned EAGAIN, which is an API violation.", __func__);
                    decoder->packetPending = 1;
                    av_packet_move_ref(&decoder->packet, &packet);
                }
            }
            av_packet_unref(&packet);
        }
    }

    return ret;
}

int Stream::queueFrameToFrameQueue(AVFrame *srcFrame, double pts, double duration, int64_t pos, int serial) {
    ALOGD(STREAM_TAG, "%s pts=%lf duration=%lf pos=%lld serial=%d", __func__, pts, duration, pos, serial);

    Frame *frame;

    if (!(frame = videoState->videoFrameQueue.peekWritable())) {
        return NEGATIVE(S_NOT_FRAME_WRITEABLE);
    }

    frame->sampleAspectRatio = srcFrame->sample_aspect_ratio;
    frame->uploaded = 0;

    frame->width = srcFrame->width;
    frame->height = srcFrame->height;
    frame->format = srcFrame->format;

    frame->pts = pts;
    frame->duration = duration;
    frame->pos = pos;
    frame->serial = serial;

    if (surface) {
        surface->setVideoSize(frame->width, frame->height, frame->sampleAspectRatio);
    }

    av_frame_move_ref(frame->frame, srcFrame);
    videoState->videoFrameQueue.push();
    return POSITIVE;
}

void Stream::checkExternalClockSpeed() {
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

double Stream::getFrameDuration(const Frame *current, const Frame *next) {
    if (current->serial == next->serial) {
        double duration = next->pts - current->pts;
        ALOGD(STREAM_TAG, "%s current->pts = %f next->pts = %f duration = %f maxFrameDuration = %f", __func__, current->pts, next->pts, duration, videoState->maxFrameDuration);
        if (isnan(duration) || duration <= 0 || duration > videoState->maxFrameDuration) {
            return current->duration;
        } else {
            return duration;
        }
    } else {
        return 0.0;
    }
}

double Stream::getComputeTargetDelay(double duration) {
    double syncThreshold, diff = 0;
    /* update duration to follow master synchronisation source */
    if (getMasterSyncType() != SYNC_TYPE_VIDEO_MASTER) {
        /* if video is slave, we try to correct big delays byduplicating or deleting a frame */
        double videoClock = videoState->videoClock.getClock();
        double masterClock = getMasterClock();

        diff = videoClock - masterClock;

        ALOGD(STREAM_TAG, "videoClock = %lf masterClock = %lf ", videoClock, masterClock);

        // skip or repeat frame. We take into account the duration to compute the threshold. I still don't know
        // if it is the best guess */
        // 0.04 ~ 0.1
        syncThreshold = FFMAX(SYNC_THRESHOLD_MIN, FFMIN(SYNC_THRESHOLD_MAX, duration));

        ALOGD(STREAM_TAG, "diff = %lf syncThreshold[0.04,0.1] = %lf ", diff, syncThreshold);

        if (!isnan(diff) && fabs(diff) < videoState->maxFrameDuration) {
            if (diff <= -syncThreshold) {
                duration = FFMAX(0, duration + diff);
            } else if (diff >= syncThreshold && duration > SYNC_FRAMEDUP_THRESHOLD) {
                duration = duration + diff;
            } else if (diff >= syncThreshold) {
                duration = 2 * duration;
            }
        }
    }
    ALOGD(STREAM_TAG, "%s duration = %f ", __func__, duration);
    return duration;
}

/* update current video pts */
void Stream::updateVideoClockPts(double pts, int64_t pos, int serial) {
    ALOGD(STREAM_TAG, "%s pts = %lf pos = %lld serial = %d", __func__, pts, pos, serial);
    videoState->videoClock.setClock(pts, serial);
    videoState->exitClock.syncClockToSlave(&videoState->videoClock);
}

VideoState *Stream::getVideoState() const {
    return videoState;
}

void Stream::stepToNextFrame() {
    ALOGD(STREAM_TAG, "%s", __func__);
    /* if the stream is paused unpause it, then stepFrame */
    if (videoState) {
        if (videoState->paused) {
            streamTogglePause();
        }
        videoState->stepFrame = 1;
    }
}

int Stream::togglePause() {
    ALOGD(STREAM_TAG, "%s", __func__);
    if (videoState) {
        streamTogglePause();
        videoState->stepFrame = 0;
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int Stream::streamTogglePause() {
    if (videoState) {
        if (videoState->paused) {
            videoState->frameTimer += (av_gettime_relative() * 1.0F / AV_TIME_BASE - videoState->videoClock.lastUpdatedTime);
            if (videoState->readPauseReturn != AVERROR(ENOSYS)) {
                videoState->videoClock.paused = 0;
            }
            videoState->videoClock.setClock(videoState->videoClock.getClock(), videoState->videoClock.serial);
        }
        videoState->exitClock.setClock(videoState->exitClock.getClock(), videoState->exitClock.serial);
        videoState->paused = videoState->audioClock.paused = videoState->videoClock.paused = videoState->exitClock.paused = !videoState->paused;
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

int Stream::forceRefresh() {
    if (videoState) {
        videoState->forceRefresh = 1;
        return POSITIVE;
    }
    return NEGATIVE(S_NULL);
}

void Stream::setOptions(Options *options) {
    Stream::options = options;
}

void Stream::setMsgQueue(MessageQueue *msgQueue) {
    Stream::msgQueue = msgQueue;
}

void Stream::setAudio(Audio *audio) {
    Stream::audio = audio;
}

void Stream::setSurface(Surface *surface) {
    Stream::surface = surface;
}

int Stream::getStartupVolume() {
    if (options->audioStartupVolume < 0) {
        ALOGD(STREAM_TAG, "%s -volume=%d < 0, setting to 0", __func__, options->audioStartupVolume);
    }
    if (options->audioStartupVolume > 100) {
        ALOGD(STREAM_TAG, "%s -volume=%d > 100 0, setting to 100", __func__, options->audioStartupVolume);
    }
    options->audioStartupVolume = av_clip(options->audioStartupVolume, 0, 100);
    options->audioStartupVolume = av_clip(MIX_MAX_VOLUME * options->audioStartupVolume / 100, 0, MIX_MAX_VOLUME);
    return options->audioStartupVolume;
}

int64_t Stream::getValidChannelLayout(uint64_t channelLayout, int channels) {
    if (channelLayout && av_get_channel_layout_nb_channels(channelLayout) == channels) {
        return channelLayout;
    } else {
        return 0;
    }
}

#if CONFIG_AVFILTER

int Stream::configureAudioFilters(const char *afilters, int forceOutputFormat) {
    static const enum AVSampleFormat sample_fmts[] = {AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE};
    int sample_rates[2] = {0, -1};
    int64_t channel_layouts[2] = {0, -1};
    int channels[2] = {0, -1};
    AVFilterContext *filterSrc = nullptr, *filterSink = nullptr;
    char aresample_swr_opts[512] = "";
    AVDictionaryEntry *e = nullptr;
    char asrc_args[256];
    int ret;

    avfilter_graph_free(&videoState->agraph);

    if (!(videoState->agraph = avfilter_graph_alloc())) {
        return NEGATIVE(S_NOT_MEMORY);
    }
    videoState->agraph->nb_threads = options->filterNumberThreads;

    while ((e = av_dict_get(options->swrDict, "", e, AV_DICT_IGNORE_SUFFIX))) {
        av_strlcatf(aresample_swr_opts, sizeof(aresample_swr_opts), "%s=%s:", e->key, e->value);
    }
    if (strlen(aresample_swr_opts)) {
        aresample_swr_opts[strlen(aresample_swr_opts) - 1] = '\0';
    }
    av_opt_set(videoState->agraph, "aresample_swr_opts", aresample_swr_opts, 0);

    ret = snprintf(asrc_args, sizeof(asrc_args), "sample_rate=%d:sample_fmt=%s:channels=%d:time_base=%d/%d", videoState->audioFilterSrc.freq, av_get_sample_fmt_name(videoState->audioFilterSrc.fmt), videoState->audioFilterSrc.channels, 1, videoState->audioFilterSrc.freq);

    if (videoState->audioFilterSrc.channel_layout) {
        snprintf(asrc_args + ret, sizeof(asrc_args) - ret, ":channel_layout=0x%lld", videoState->audioFilterSrc.channel_layout);
    }

    ret = avfilter_graph_create_filter(&filterSrc, avfilter_get_by_name("abuffer"), "splayer_abuffer", asrc_args, nullptr, videoState->agraph);
    if (ret < 0) {
        goto end;
    }

    ret = avfilter_graph_create_filter(&filterSink, avfilter_get_by_name("abuffersink"), "splayer_abuffersink", nullptr, nullptr, videoState->agraph);
    if (ret < 0) {
        goto end;
    }

    if ((ret = av_opt_set_int_list(filterSink, "sample_fmts", sample_fmts, AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN)) < 0) {
        goto end;
    }

    if ((ret = av_opt_set_int(filterSink, "all_channel_counts", 1, AV_OPT_SEARCH_CHILDREN)) < 0) {
        goto end;
    }

    if (forceOutputFormat) {
        channel_layouts[0] = videoState->audioTarget.channel_layout;
        channels[0] = videoState->audioTarget.channels;
        sample_rates[0] = videoState->audioTarget.freq;
        if ((ret = av_opt_set_int(filterSink, "all_channel_counts", 0, AV_OPT_SEARCH_CHILDREN)) < 0) {
            goto end;
        }
        if ((ret = av_opt_set_int_list(filterSink, "channel_layouts", channel_layouts, -1, AV_OPT_SEARCH_CHILDREN)) < 0) {
            goto end;
        }
        if ((ret = av_opt_set_int_list(filterSink, "channel_counts", channels, -1, AV_OPT_SEARCH_CHILDREN)) < 0) {
            goto end;
        }
        if ((ret = av_opt_set_int_list(filterSink, "sample_rates", sample_rates, -1, AV_OPT_SEARCH_CHILDREN)) < 0) {
            goto end;
        }
    }

    if ((ret = configureFilterGraph(videoState->agraph, afilters, filterSrc, filterSink)) < 0) {
        goto end;
    }

    videoState->inAudioFilter = filterSrc;
    videoState->outAudioFilter = filterSink;

    end:
    if (ret < 0) {
        avfilter_graph_free(&videoState->agraph);
    }

    return ret;
}

int Stream::configureFilterGraph(AVFilterGraph *graph, const char *filterGraph, AVFilterContext *srcFilterContext, AVFilterContext *sinkFilterContext) {
    int ret, i;
    int nb_filters = graph->nb_filters;
    AVFilterInOut *outputs = nullptr, *inputs = nullptr;

    if (filterGraph) {
        outputs = avfilter_inout_alloc();
        inputs = avfilter_inout_alloc();

        if (!outputs || !inputs) {
            avfilter_inout_free(&outputs);
            avfilter_inout_free(&inputs);
            return NEGATIVE(S_NOT_MEMORY);
        }

        outputs->name = av_strdup("in");
        outputs->filter_ctx = srcFilterContext;
        outputs->pad_idx = 0;
        outputs->next = nullptr;

        inputs->name = av_strdup("out");
        inputs->filter_ctx = sinkFilterContext;
        inputs->pad_idx = 0;
        inputs->next = nullptr;

        if ((ret = avfilter_graph_parse_ptr(graph, filterGraph, &inputs, &outputs, nullptr)) < 0) {
            avfilter_inout_free(&outputs);
            avfilter_inout_free(&inputs);
            return ret;
        }

    } else {

        if ((ret = avfilter_link(srcFilterContext, 0, sinkFilterContext, 0)) < 0) {
            avfilter_inout_free(&outputs);
            avfilter_inout_free(&inputs);
            return ret;
        }
    }

    /* Reorder the filters to ensure that inputs of the custom filters are merged first */
    for (i = 0; i < graph->nb_filters - nb_filters; i++) {
        FFSWAP(AVFilterContext*, graph->filters[i], graph->filters[i + nb_filters]);
    }

    ret = avfilter_graph_config(graph, nullptr);
    return ret;
}

int Stream::configureVideoFilters(AVFilterGraph *filterGraph, VideoState *is, const char *filters, AVFrame *frame) {
    AVFilterContext *filterSrc = nullptr;
    AVFilterContext *filterOut = nullptr;
    AVFilterContext *lastFilter = nullptr;
    AVCodecParameters *codecParameters = is->videoStream->codecpar;
    AVRational fr = av_guess_frame_rate(is->formatContext, is->videoStream, nullptr);
    AVDictionaryEntry *e = nullptr;
    char swsFlagsStr[512] = "";
    char bufferSrcArgs[256];
    int ret;

    while ((e = av_dict_get(options->swsDict, "", e, AV_DICT_IGNORE_SUFFIX))) {
        if (!strcmp(e->key, "sws_flags")) {
            av_strlcatf(swsFlagsStr, sizeof(swsFlagsStr), "%s=%s:", "flags", e->value);
        } else {
            av_strlcatf(swsFlagsStr, sizeof(swsFlagsStr), "%s=%s:", e->key, e->value);
        }
    }
    if (strlen(swsFlagsStr)) {
        swsFlagsStr[strlen(swsFlagsStr) - 1] = '\0';
    }

    filterGraph->scale_sws_opts = av_strdup(swsFlagsStr);

    snprintf(bufferSrcArgs, sizeof(bufferSrcArgs), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d", frame->width, frame->height, frame->format, is->videoStream->time_base.num, is->videoStream->time_base.den, codecParameters->sample_aspect_ratio.num, FFMAX(codecParameters->sample_aspect_ratio.den, 1));

    if (fr.num && fr.den) {
        av_strlcatf(bufferSrcArgs, sizeof(bufferSrcArgs), ":frame_rate=%d/%d", fr.num, fr.den);
    }

    if ((ret = avfilter_graph_create_filter(&filterSrc, avfilter_get_by_name("buffer"), "splayer_buffer", bufferSrcArgs, nullptr, filterGraph)) < 0) {
        return ret;
    }

    if ((ret = avfilter_graph_create_filter(&filterOut, avfilter_get_by_name("buffersink"), "splayer_buffersink", nullptr, nullptr, filterGraph)) < 0) {
        return ret;
    }

    if (surface) {
        AVPixelFormat *pixelFormat = surface->getPixelFormatsArray();
        if ((ret = av_opt_set_int_list(filterOut, "pix_fmts", pixelFormat, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN)) < 0) {
            return ret;
        }
    }

    lastFilter = filterOut;

/* Note: this macro adds a filter before the lastly added filter, so the
 * processing order of the filters is in reverse */
#define INSERT_FILT(name, arg) do {                                          \
    AVFilterContext *filt_ctx;                                               \
                                                                             \
    ret = avfilter_graph_create_filter(&filt_ctx,                            \
                                       avfilter_get_by_name(name),           \
                                       "splayer_" name, arg, NULL, filterGraph);    \
    if (ret < 0)                                                             \
        return ret;                                                           \
                                                                             \
    ret = avfilter_link(filt_ctx, 0, lastFilter, 0);                        \
    if (ret < 0)                                                             \
        return ret;                                                           \
                                                                             \
    lastFilter = filt_ctx;                                                  \
} while (0)

    if (options->autoRotate) {
        double theta = get_rotation(is->videoStream);

        if (fabs(theta - 90) < 1.0) {
            INSERT_FILT("transpose", "clock");
        } else if (fabs(theta - 180) < 1.0) {
            INSERT_FILT("hflip", nullptr);
            INSERT_FILT("vflip", nullptr);
        } else if (fabs(theta - 270) < 1.0) {
            INSERT_FILT("transpose", "cclock");
        } else if (fabs(theta) > 1.0) {
            char rotate_buf[64];
            snprintf(rotate_buf, sizeof(rotate_buf), "%f*PI/180", theta);
            INSERT_FILT("rotate", rotate_buf);
        }
    }

    if ((ret = configureFilterGraph(filterGraph, filters, filterSrc, lastFilter)) < 0) {
        return ret;
    }

    is->videoInFilter = filterSrc;
    is->videoOutFilter = filterOut;

    return ret;
}

#endif
