#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "modernize-use-auto"
#pragma ide diagnostic ignored "OCDFAInspection"

#include <FFPlay.h>

#include "FFPlay.h"

FFPlay::FFPlay() {
    ALOGD(__func__);
    avMutex = new Mutex();
    vfMutex = new Mutex();
    msgQueue = new MessageQueue();
}

FFPlay::~FFPlay() {
    ALOGD(__func__);
    delete pipeline;
    delete aOut;
    delete avMutex;
    delete vfMutex;
    delete msgQueue;
}

void FFPlay::setAOut(AOut *aOut) {
    ALOGD(__func__);
    FFPlay::aOut = aOut;
}

void FFPlay::setVOut(VOut *vOut) {
    ALOGD(__func__);
    FFPlay::vOut = vOut;
}

void FFPlay::setPipeline(Pipeline *pipeline) {
    ALOGD(__func__);
    FFPlay::pipeline = pipeline;
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

int FFPlay::prepareAsync(const char *fileName) {
    showVersionsAndOptions();

    av_init_packet(&flushPacket);
    flushPacket.data = (uint8_t *) &flushPacket;

    inputFileName = fileName != nullptr ? av_strdup(fileName) : nullptr;

    videoState = streamOpen();

    if (!videoState) {
        return NEGATIVE(S_NOT_MEMORY);
    }

    return POSITIVE;
}

void FFPlay::showVersionsAndOptions() {
    ALOGI("===== versions =====");
    ALOGI("%-*s: %s", VERSION_MODULE_FILE_NAME_LENGTH, "FFmpeg", av_version_info());
    ALOGI("%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "libavutil", avutil_version());
    ALOGI("%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "libavcodec", avcodec_version());
    ALOGI("%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "libavformat", avformat_version());
    ALOGI("%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "libswscale", swscale_version());
    ALOGI("%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "libswresample", swresample_version());

    ALOGI("===== options =====");
    showDict("player-opts", playerOpts);
    showDict("format-opts", formatOpts);
    showDict("codec-opts ", codecOpts);
    showDict("sws-opts   ", swsDict);
    showDict("swr-opts   ", swrOpts);
}

int FFPlay::getMsg(Message *msg, bool block) {
    while (true) {
        bool continueWaitNextMsg = false;
        int ret = msgQueue->getMsg(msg, block);
        ALOGD("%s get msg ret=%d", __func__, ret);
        if (ret != POSITIVE) {
            return ret;
        }
        switch (msg->what) {
            case Message::MSG_PREPARED:
                ALOGD("%s MSG_PREPARED", __func__);
                break;
            case Message::MSG_COMPLETED:
                ALOGD("%s MSG_COMPLETED", __func__);
                break;
            case Message::MSG_SEEK_COMPLETE:
                ALOGD("%s MSG_SEEK_COMPLETE", __func__);
                break;
            case Message::REQ_START:
                ALOGD("%s REQ_START", __func__);
                break;
            case Message::REQ_PAUSE:
                ALOGD("%s REQ_PAUSE", __func__);
                break;
            case Message::REQ_SEEK:
                ALOGD("%s REQ_SEEK", __func__);
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
        ALOGI("%-*s: %-*s = %s\n", 12, tag, 28, t->key, t->value);
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

    if (!inputFileName) {
        ALOGE("%s input file name is null", __func__);
        return nullptr;
    }

    auto *is = new VideoState();
    if (!is) {
        ALOGD("%s create video state oom", __func__);
        return nullptr;
    }

    if (is->videoFrameQueue.frameQueueInit(&is->videoPacketQueue, VIDEO_QUEUE_SIZE, 1) < 0) {
        ALOGE("%s video frame packetQueue init fail", __func__);
        streamClose();
        return nullptr;
    }

    if (is->audioFrameQueue.frameQueueInit(&is->audioPacketQueue, AUDIO_QUEUE_SIZE, 0) < 0) {
        ALOGE("%s audio frame packetQueue init fail", __func__);
        streamClose();
        return nullptr;
    }

    if (is->subtitleFrameQueue.frameQueueInit(&is->subtitlePacketQueue, SUBTITLE_QUEUE_SIZE, 0) < 0) {
        ALOGE("%s subtitle frame packetQueue init fail", __func__);
        streamClose();
        return nullptr;
    }

    if (is->videoPacketQueue.packetQueueInit(&flushPacket) < 0 ||
        is->audioPacketQueue.packetQueueInit(&flushPacket) < 0 ||
        is->subtitlePacketQueue.packetQueueInit(&flushPacket) < 0) {
        ALOGE("%s packet packetQueue init fail", __func__);
        streamClose();
        return nullptr;
    }

    if (!(is->continueReadThread = new Mutex())) {
        ALOGE("%s create continue read thread mutex fail", __func__);
        streamClose();
        return nullptr;
    }

    if (is->videoClock.initClock(&is->videoPacketQueue.serial) < 0 ||
        is->audioClock.initClock(&is->audioPacketQueue.serial) < 0 ||
        is->exitClock.initClock(&is->subtitlePacketQueue.serial) < 0) {
        ALOGE("%s init clock fail", __func__);
        streamClose();
        return nullptr;
    }

    is->fileName = av_strdup(inputFileName);
    is->audioClockSerial = -1;
    is->inputFormat = inputFormat;
    is->yTop = 0;
    is->xLeft = 0;
    is->audioVolume = getStartupVolume();
    is->muted = 0;
    is->avSyncType = avSyncType;
    is->readThread = new Thread(innerReadThread, this, "readThread");

    if (!is->readThread) {
        ALOGE("%s create read thread fail", __func__);
        streamClose();
        return nullptr;
    }

    return is;
}

int FFPlay::getStartupVolume() {
    if (startupVolume < 0) {
        ALOGD("%s -volume=%d < 0, setting to 0", __func__, startupVolume);
    }
    if (startupVolume > 100) {
        ALOGD("%s -volume=%d > 100 0, setting to 100", __func__, startupVolume);
    }
    startupVolume = av_clip(startupVolume, 0, 100);
    startupVolume = av_clip(MIX_MAX_VOLUME * startupVolume / 100, 0, MIX_MAX_VOLUME);
    return startupVolume;
}

void FFPlay::streamClose() {
    if (videoState) {

        /* XXX: use a special url_shutdown call to abort parse cleanly */

        videoState->abortRequest = 0;

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
        avformat_close_input(&videoState->ic);

        // free all packet
        videoState->videoPacketQueue.packetQueueDestroy();
        videoState->audioPacketQueue.packetQueueDestroy();
        videoState->subtitlePacketQueue.packetQueueDestroy();

        // free all frame
        videoState->videoFrameQueue.frameQueueDestroy();
        videoState->audioFrameQueue.frameQueueDestroy();
        videoState->subtitleFrameQueue.frameQueueDestroy();

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
    ALOGD(__func__);
    if (!videoState) {
        return NEGATIVE(S_NULL);
    }
    AVFormatContext *formatContext = nullptr;
    AVPacket packet1, *packet = &packet1;
    Mutex *waitMutex = new Mutex();
    AVDictionaryEntry *dictionaryEntry;
    int streamIndex[AVMEDIA_TYPE_NB] = {-1};
    int packetInPlayRange = 0;
    int scanAllPmtsSet = 0;
    int ret = 0;
    int64_t streamStartTime;
    int64_t packetTimestamp;

    if (!waitMutex) {
        ALOGE("%s create wait mutex fail", __func__);
        return NEGATIVE(S_NOT_MEMORY);
    }

    videoState->lastVideoStreamIndex = videoState->videoStreamIndex = -1;
    videoState->lastAudioStreamIndex = videoState->audioStreamIndex = -1;
    videoState->lastSubtitleStreamIndex = videoState->subtitleStreamIndex = -1;
    videoState->eof = 0;

    formatContext = avformat_alloc_context();
    if (!formatContext) {
        ALOGE("%s avformat could not allocate context", __func__);
        return NEGATIVE(S_NOT_MEMORY);
    }

    formatContext->interrupt_callback.callback = decodeInterruptCallback;
    formatContext->interrupt_callback.opaque = videoState;

    if (!av_dict_get(formatOpts, SCAN_ALL_PMTS, nullptr, AV_DICT_MATCH_CASE)) {
        av_dict_set(&formatOpts, SCAN_ALL_PMTS, "1", AV_DICT_DONT_OVERWRITE);
        scanAllPmtsSet = 1;
    }

    if (inputFormatName) {
        videoState->inputFormat = av_find_input_format(inputFormatName);
    }

    if (avformat_open_input(&formatContext, videoState->fileName, videoState->inputFormat, &formatOpts) < 0) {
        ALOGE("%s avformat could not open input", __func__);
        closeReadThread(videoState, formatContext);
        return NEGATIVE(S_NOT_OPEN_INPUT);
    }

    if (msgQueue) {
        msgQueue->notifyMsg(Message::MSG_OPEN_INPUT);
    }

    if (scanAllPmtsSet) {
        av_dict_set(&formatOpts, SCAN_ALL_PMTS, nullptr, AV_DICT_MATCH_CASE);
    }

    if ((dictionaryEntry = av_dict_get(formatOpts, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
        ALOGE("%s option %s not found.", __func__, dictionaryEntry->key);
        closeReadThread(videoState, formatContext);
        return NEGATIVE_OPTION_NOT_FOUND;
    }

    videoState->ic = formatContext;

    if (genpts) {
        formatContext->flags |= AVFMT_FLAG_GENPTS;
    }

    av_format_inject_global_side_data(formatContext);

    if (findStreamInfo) {
        AVDictionary **opts = setupFindStreamInfoOpts(formatContext, codecOpts);
        int ret = avformat_find_stream_info(formatContext, opts);
        if (msgQueue) {
            msgQueue->notifyMsg(Message::MSG_FIND_STREAM_INFO);
        }
        for (int i = 0; i < formatContext->nb_streams; i++) {
            av_dict_free(&opts[i]);
        }
        av_freep(&opts);
        if (ret < 0) {
            ALOGD("%s %s: could not find codec parameters", __func__, videoState->fileName);
            closeReadThread(videoState, formatContext);
            return NEGATIVE(S_NOT_FIND_STREAM_INFO);
        }
    }

    if (formatContext->pb) {
        formatContext->pb->eof_reached = 0;
    }

    if (seekByBytes < 0) {
        seekByBytes = (formatContext->iformat->flags & AVFMT_TS_DISCONT) &&
                      strcmp(OGG, formatContext->iformat->name) != 0;
    }

    videoState->maxFrameDuration = (formatContext->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;

    if (!windowTitle && (dictionaryEntry = av_dict_get(formatContext->metadata, TITLE, nullptr, 0))) {
        windowTitle = av_asprintf("%s - %s", dictionaryEntry->value, inputFileName);
    }

    /* if seeking requested, we execute it */
    if (startTime != AV_NOPTS_VALUE) {
        int64_t timestamp = startTime;
        /* add the stream start time */
        if (formatContext->start_time != AV_NOPTS_VALUE) {
            timestamp += formatContext->start_time;
        }
        if (avformat_seek_file(formatContext, -1, INT64_MIN, timestamp, INT64_MAX, 0) < 0) {
            ALOGD("%s %s: could not seek to position %0.3f", __func__, videoState->fileName,
                  (double) timestamp / AV_TIME_BASE);
        }
    }

    videoState->realTime = isRealTime(formatContext);

    if (showStatus) {
        av_dump_format(formatContext, 0, videoState->fileName, 0);
    }

    for (int i = 0; i < formatContext->nb_streams; i++) {
        AVStream *stream = formatContext->streams[i];
        AVMediaType type = stream->codecpar->codec_type;
        stream->discard = AVDISCARD_ALL;
        if (type >= 0 && wantedStreamSpec[type] && streamIndex[type] == -1) {
            if (avformat_match_stream_specifier(formatContext, stream, wantedStreamSpec[type]) > 0) {
                streamIndex[type] = i;
            }
        }
    }

    for (int i = 0; i < AVMEDIA_TYPE_NB; i++) {
        if (wantedStreamSpec[i] && streamIndex[i] == -1) {
            ALOGD("%s Stream specifier %s does not match any stream", __func__, wantedStreamSpec[i]);
            streamIndex[i] = INT_MAX;
        }
    }

    if (!videoDisable) {
        streamIndex[AVMEDIA_TYPE_VIDEO] = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO,
                                                              streamIndex[AVMEDIA_TYPE_VIDEO], -1, nullptr, 0);
    }

    if (!audioDisable) {
        streamIndex[AVMEDIA_TYPE_AUDIO] = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO,
                                                              streamIndex[AVMEDIA_TYPE_AUDIO],
                                                              streamIndex[AVMEDIA_TYPE_VIDEO],
                                                              nullptr, 0);
    }

    if (!videoDisable && !subtitleDisable) {
        streamIndex[AVMEDIA_TYPE_SUBTITLE] = av_find_best_stream(formatContext, AVMEDIA_TYPE_SUBTITLE,
                                                                 streamIndex[AVMEDIA_TYPE_SUBTITLE],
                                                                 (streamIndex[AVMEDIA_TYPE_AUDIO] >= 0 ?
                                                                  streamIndex[AVMEDIA_TYPE_AUDIO] :
                                                                  streamIndex[AVMEDIA_TYPE_VIDEO]),
                                                                 nullptr, 0);
    }

    videoState->showMode = showMode;

    if (streamIndex[AVMEDIA_TYPE_VIDEO] >= 0) {
        AVStream *stream = formatContext->streams[streamIndex[AVMEDIA_TYPE_VIDEO]];
        AVCodecParameters *codecParameters = stream->codecpar;
        AVRational sar = av_guess_sample_aspect_ratio(formatContext, stream, nullptr);
        if (codecParameters->width) {
            // TODO
            // set_default_window_size(codecParameters->width, codecParameters->height, sar);
        }
    }

    /* open the streams */
    if (streamIndex[AVMEDIA_TYPE_AUDIO] >= 0) {
        streamComponentOpen(streamIndex[AVMEDIA_TYPE_AUDIO]);
    } else {
        videoState->avSyncType = avSyncType = AV_SYNC_VIDEO_MASTER;
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
        ALOGD("%s failed to open file '%s' or configure filter graph", __func__, videoState->fileName);
        closeReadThread(videoState, formatContext);
        return NEGATIVE(S_NOT_OPEN_FILE);
    }

    if (infiniteBuffer < 0 && videoState->realTime) {
        infiniteBuffer = 1;
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

    if (autoResume) {
        if (msgQueue) {
            msgQueue->notifyMsg(Message::REQ_START);
        }
        autoResume = 0;
    }

    /* offset should be seeked */
    if (seekAtStart > 0) {
        // ffp_seek_to_l((long) (seekAtStart));
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
            if (avformat_seek_file(videoState->ic, -1, seekMin, seekTarget, seekMax, videoState->seekFlags) < 0) {
                ALOGD("%s %s: error while seeking", __func__, videoState->ic->url);
            } else {
                if (videoState->audioStreamIndex >= 0) {
                    videoState->audioPacketQueue.packetQueueFlush();
                    videoState->audioPacketQueue.packetQueuePut(&flushPacket);
                }
                if (videoState->subtitleStreamIndex >= 0) {
                    videoState->subtitlePacketQueue.packetQueueFlush();
                    videoState->subtitlePacketQueue.packetQueuePut(&flushPacket);
                }
                if (videoState->videoStreamIndex >= 0) {
                    videoState->videoPacketQueue.packetQueueFlush();
                    videoState->videoPacketQueue.packetQueuePut(&flushPacket);
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
                videoState->videoPacketQueue.packetQueuePut(&copy);
                videoState->videoPacketQueue.packetQueuePutNullPacket(videoState->videoStreamIndex);
            }
            videoState->queueAttachmentsReq = 0;
        }

        /* if the packetQueue are full, no need to read more */
        bool cond1 = infiniteBuffer < 1;
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
        bool audioSeekCond = !videoState->audioStream || (videoState->audioDecoder.finished == videoState->audioPacketQueue.serial && videoState->audioFrameQueue.frameQueueNumberRemaining() == 0);
        // 未初始化视频流 或者 解码结束 同时 无可用帧
        bool videoSeekCond = !videoState->videoStream || (videoState->videoDecoder.finished == videoState->videoPacketQueue.serial && videoState->videoFrameQueue.frameQueueNumberRemaining() == 0);
        if (notPaused && audioSeekCond && videoSeekCond) {
            if (loop != 1 && (!loop || --loop)) {
                streamSeek(startTime != AV_NOPTS_VALUE ? startTime : 0, 0, 0);
            } else if (autoExit) {
                return NEGATIVE(NEGATIVE_EOF);
            }
        }

        // 读取帧
        ret = av_read_frame(formatContext, packet);

        if (ret < 0) {
            // 0 if OK, < 0 on error or end of file
            if ((ret == AVERROR_EOF || avio_feof(formatContext->pb)) && !videoState->eof) {
                if (videoState->videoStreamIndex >= 0) {
                    videoState->videoPacketQueue.packetQueuePutNullPacket(videoState->videoStreamIndex);
                }
                if (videoState->audioStreamIndex >= 0) {
                    videoState->audioPacketQueue.packetQueuePutNullPacket(videoState->audioStreamIndex);
                }
                if (videoState->subtitleStreamIndex >= 0) {
                    videoState->subtitlePacketQueue.packetQueuePutNullPacket(videoState->subtitleStreamIndex);
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
        double _startTime = (double) (startTime != AV_NOPTS_VALUE ? startTime : 0) / 1000000;
        double _duration = (double) duration / 1000000;
        
        packetInPlayRange = (duration == AV_NOPTS_VALUE) || (_diffTime - _startTime <= _duration);

        if (packet->stream_index == videoState->audioStreamIndex && packetInPlayRange) {
            videoState->audioPacketQueue.packetQueuePut(packet);
            ALOGD("%s audio memorySize=%d serial=%d packetSize=%d duration=%lld abortRequest=%d", __func__,
                  videoState->audioPacketQueue.memorySize,
                  videoState->audioPacketQueue.serial,
                  videoState->audioPacketQueue.packetSize,
                  duration,
                  videoState->audioPacketQueue.abortRequest);
        } else if (packet->stream_index == videoState->videoStreamIndex && packetInPlayRange &&
                   !(videoState->videoStream->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
            videoState->videoPacketQueue.packetQueuePut(packet);
            ALOGD("%s video memorySize=%d serial=%d packetSize=%d duration=%lld abortRequest=%d", __func__,
                  videoState->videoPacketQueue.memorySize,
                  videoState->videoPacketQueue.serial,
                  videoState->videoPacketQueue.packetSize,
                  duration,
                  videoState->videoPacketQueue.abortRequest);
        } else if (packet->stream_index == videoState->subtitleStreamIndex && packetInPlayRange) {
            videoState->subtitlePacketQueue.packetQueuePut(packet);
            ALOGD("%s subtitle memorySize=%d serial=%d packetSize=%d duration=%lld abortRequest=%d", __func__,
                  videoState->subtitlePacketQueue.memorySize,
                  videoState->subtitlePacketQueue.serial,
                  videoState->subtitlePacketQueue.packetSize,
                  duration,
                  videoState->subtitlePacketQueue.abortRequest);
        } else {
            av_packet_unref(packet);
        }
    } // for end
    return POSITIVE;
}

void FFPlay::closeReadThread(const VideoState *is, AVFormatContext *&formatContext) const {
    if (formatContext && !is->ic) {
        avformat_close_input(&formatContext);
    }
}

int FFPlay::isRealTime(AVFormatContext *s) {
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
    ALOGD("%s streamIndex=%d", __func__, streamIndex);
    AVFormatContext *formatContext = videoState->ic;
    AVCodecContext *codecContext;
    AVCodec *codec;
    const char *forcedCodecName = nullptr;
    AVDictionary *opts = nullptr;
    AVDictionaryEntry *t = nullptr;
    int sampleRate, nbChannels;
    int64_t channelLayout;
    int streamLowres = lowres;

    if (streamIndex < 0 || streamIndex >= formatContext->nb_streams) {
        return NEGATIVE(S_INVALID_STREAM_INDEX);
    }

    codecContext = avcodec_alloc_context3(nullptr);
    if (!codecContext) {
        return NEGATIVE(S_NOT_MEMORY);
    }

    if (avcodec_parameters_to_context(codecContext, formatContext->streams[streamIndex]->codecpar) < 0) {
        avcodec_free_context(&codecContext);
        return NEGATIVE(S_CODEC_PARAMS_CONTEXT);
    }

    codecContext->pkt_timebase = formatContext->streams[streamIndex]->time_base;

    codec = avcodec_find_decoder(codecContext->codec_id);

    switch (codecContext->codec_type) {
        case AVMEDIA_TYPE_AUDIO   :
            videoState->lastAudioStreamIndex = streamIndex;
            forcedCodecName = audioCodecName;
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            videoState->lastSubtitleStreamIndex = streamIndex;
            forcedCodecName = subtitleCodecName;
            break;
        case AVMEDIA_TYPE_VIDEO   :
            videoState->lastVideoStreamIndex = streamIndex;
            forcedCodecName = videoCodecName;
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
            ALOGD("%s No codec could be found with name '%s'", __func__, forcedCodecName);
        } else {
            ALOGD("%sNo decoder could be found for codec %s", __func__, avcodec_get_name(codecContext->codec_id));
        }
        avcodec_free_context(&codecContext);
        return NEGATIVE(S_EINVAL);
    }

    codecContext->codec_id = codec->id;

    if (streamLowres > codec->max_lowres) {
        ALOGD("%s The maximum value for lowres supported by the decoder is %d", __func__,
              codec->max_lowres);
        streamLowres = codec->max_lowres;
    }
    codecContext->lowres = streamLowres;

    if (fast) {
        codecContext->flags2 |= AV_CODEC_FLAG2_FAST;
    }

    opts = filter_codec_opts(codecOpts, codecContext->codec_id, formatContext, formatContext->streams[streamIndex],
                             codec);

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
        return NEGATIVE(S_NOT_OPEN_CODEC);
    }

    if ((t = av_dict_get(opts, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
        ALOGE("%s Option %s not found.", __func__, t->key);
        return NEGATIVE(S_OPTION_NOT_FOUND);
    }

    videoState->eof = 0;

    formatContext->streams[streamIndex]->discard = AVDISCARD_DEFAULT;

    switch (codecContext->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            videoState->audioStreamIndex = streamIndex;
            videoState->audioStream = formatContext->streams[streamIndex];
            videoState->audioDecoder.decoderInit(codecContext,
                                                 &videoState->audioPacketQueue,
                                                 videoState->continueReadThread);
            if ((videoState->ic->iformat->flags & (AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK)) &&
                !videoState->ic->iformat->read_seek) {
                videoState->audioDecoder.startPts = videoState->audioStream->start_time;
                videoState->audioDecoder.startPtsTb = videoState->audioStream->time_base;
            }
            if (videoState->audioDecoder.decoderStart(innerSubtitleThread, this) < 0) {
                av_dict_free(&opts);
                return NEGATIVE(S_NOT_AUDIO_DECODE_START);
            }
            break;
        case AVMEDIA_TYPE_VIDEO:
            videoState->videoStreamIndex = streamIndex;
            videoState->videoStream = formatContext->streams[streamIndex];
            videoState->videoDecoder.decoderInit(codecContext,
                                                 &videoState->videoPacketQueue,
                                                 videoState->continueReadThread);
            if (videoState->videoDecoder.decoderStart(innerVideoThread, this) < 0) {
                av_dict_free(&opts);
                return NEGATIVE(S_NOT_VIDEO_DECODE_START);
            }
            videoState->queueAttachmentsReq = 1;
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            videoState->subtitleStreamIndex = streamIndex;
            videoState->subtitleStream = formatContext->streams[streamIndex];
            videoState->subtitleDecoder.decoderInit(codecContext,
                                                    &videoState->subtitlePacketQueue,
                                                    videoState->continueReadThread);
            if (videoState->subtitleDecoder.decoderStart(innerSubtitleThread, this) < 0) {
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
    AVFormatContext *ic = videoState->ic;
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
        case AV_SYNC_VIDEO_MASTER:
            val = videoState->videoClock.getClock();
            break;
        case AV_SYNC_AUDIO_MASTER:
            val = videoState->audioClock.getClock();
            break;
        default:
            val = videoState->exitClock.getClock();
            break;
    }
    return val;
}

int FFPlay::getMasterSyncType() {
    if (videoState && videoState->avSyncType == AV_SYNC_VIDEO_MASTER) {
        if (videoState->videoStream) {
            return AV_SYNC_VIDEO_MASTER;
        } else {
            return AV_SYNC_AUDIO_MASTER;
        }
    } else if (videoState && videoState->avSyncType == AV_SYNC_AUDIO_MASTER) {
        if (videoState->audioStream) {
            return AV_SYNC_AUDIO_MASTER;
        } else {
            return AV_SYNC_EXTERNAL_CLOCK;
        }
    } else {
        return AV_SYNC_EXTERNAL_CLOCK;
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
    ALOGD(__func__);

    VideoState *is = videoState;
    AVFrame *frame = av_frame_alloc();
    double pts;
    double duration;
    int ret;
    AVRational tb = is->videoStream->time_base;
    AVRational frame_rate = av_guess_frame_rate(is->ic, is->videoStream, nullptr);

    if (!frame) {
        return NEGATIVE(S_NOT_MEMORY);
    }

    for (;;) {
        ret = getVideoFrame(frame);
        if (ret < 0) {
            av_frame_free(&frame);
            return NEGATIVE(S_NOT_GET_VIDEO_FRAME);
        }
        if (!ret) {
            continue;
        }

        duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational) {frame_rate.den, frame_rate.num}) : 0);
        pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
        ret = queuePicture(frame, pts, duration, frame->pkt_pos, is->videoDecoder.packetSerial);
        av_frame_unref(frame);

        if (ret < 0) {
            av_frame_free(&frame);
            return NEGATIVE(S_NOT_QUEUE_PICTURE);
        }
    }
    return POSITIVE;
}

int FFPlay::subtitleThread() {
    ALOGD(__func__);
    return POSITIVE;
}

int FFPlay::audioThread() {
    ALOGD(__func__);
    return POSITIVE;
}

int FFPlay::getVideoFrame(AVFrame *frame) {
    int gotPicture;

    if ((gotPicture = decoderDecodeFrame(&videoState->videoDecoder, frame, nullptr)) < 0) {
        return NEGATIVE(S_NOT_DECODE_FRAME);
    }

    if (gotPicture) {
        double dpts = NAN;

        if (frame->pts != AV_NOPTS_VALUE) {
            dpts = av_q2d(videoState->videoStream->time_base) * frame->pts;
        }

        frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(videoState->ic, videoState->videoStream, frame);

        if (frameDrop > 0 || (frameDrop && getMasterSyncType() != AV_SYNC_VIDEO_MASTER)) {
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
    int ret = AVERROR(EAGAIN);
    for (;;) {
        AVPacket packet;
        if (decoder->packetQueue->serial == decoder->packetSerial) {
            do {
                if (decoder->packetQueue->abortRequest) {
                    return NEGATIVE(S_NOT_ABORT_REQUEST);
                }
                switch (decoder->codecContext->codec_type) {
                    case AVMEDIA_TYPE_VIDEO:
                        ret = avcodec_receive_frame(decoder->codecContext, frame);
                        if (ret >= 0) {
                            if (decoderReorderPts == -1) {
                                frame->pts = frame->best_effort_timestamp;
                            } else if (!decoderReorderPts) {
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
                if (decoder->packetQueue->packetQueueGet(&packet, 1, &decoder->packetSerial) < 0) {
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
                    ALOGE("%s Receive_frame and send_packet both returned EAGAIN, which is an API violation.",
                          __func__);
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
    ALOGD("%s pts=%lf duration=%lf pos=%ld serial=%d", __func__, pts, duration, pos, serial);
    return POSITIVE;
}


#pragma clang diagnostic pop
#pragma clang diagnostic pop