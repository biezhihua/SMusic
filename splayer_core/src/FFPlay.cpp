#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "modernize-use-auto"

#include <FFPlay.h>

#include "FFPlay.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "OCDFAInspection"

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
    return NEGATIVE(ERROR_UNKNOWN);
}

int FFPlay::shutdown() {
    // TODO
    waitStop();
    return NEGATIVE(ERROR_UNKNOWN);
}

int FFPlay::waitStop() {
    // TODO
    return NEGATIVE(ERROR_UNKNOWN);
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

/* this thread gets the stream from the disk or the network */
static int innerReadThread(void *arg) {
    auto *play = static_cast<FFPlay *>(arg);
    if (play) {
        return play->readThread();
    }
    return 1;
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
        ALOGE("%s video frame queue init fail", __func__);
        streamClose();
        return nullptr;
    }

    if (is->audioFrameQueue.frameQueueInit(&is->audioPacketQueue, AUDIO_QUEUE_SIZE, 0) < 0) {
        ALOGE("%s audio frame queue init fail", __func__);
        streamClose();
        return nullptr;
    }

    if (is->subtitleFrameQueue.frameQueueInit(&is->subtitlePacketQueue, SUBTITLE_QUEUE_SIZE, 0) < 0) {
        ALOGE("%s subtitle frame queue init fail", __func__);
        streamClose();
        return nullptr;
    }

    if (is->videoPacketQueue.packetQueueInit(&flushPacket) < 0 ||
        is->audioPacketQueue.packetQueueInit(&flushPacket) < 0 ||
        is->subtitlePacketQueue.packetQueueInit(&flushPacket) < 0) {
        ALOGE("%s packet queue init fail", __func__);
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
    is->readTid = new Thread(innerReadThread, this, "readThread");

    if (!is->readTid) {
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

        if (videoState->readTid) {
            videoState->readTid->waitThread();
        }

        // close all streams
        if (videoState->audioStreamIndex >= 0) {
            streamComponentClose(videoState->audioStream);
        }
        if (videoState->videoStreamIndex >= 0) {
            streamComponentClose(videoState->videoStream);
        }
        if (videoState->subtitleStreamIndex >= 0) {
            streamComponentClose(videoState->subtitleStream);
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

int FFPlay::readThread() {
    ALOGD(__func__);
    if (!videoState) {
        return NEGATIVE(S_NULL);
    }
    AVFormatContext *formatContext = nullptr;
    AVPacket packet1, *packet = &packet1;
    Mutex *waitMutex = new Mutex();
    AVDictionaryEntry *t;
    int err, i, ret;
    int streamIndex[AVMEDIA_TYPE_NB] = {-1};
    int packetInPlayRange = 0;
    int scanAllPmtsSet = 0;
    int64_t streamStartTime;
    int64_t packetTs;

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

    err = avformat_open_input(&formatContext, videoState->fileName, videoState->inputFormat, &formatOpts);
    if (err < 0) {
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

    if ((t = av_dict_get(formatOpts, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
        ALOGE("%s option %s not found.", __func__, t->key);
        closeReadThread(videoState, formatContext);
        return NEGATIVE(ERROR_OPTION_NOT_FOUND);
    }

    videoState->ic = formatContext;

    if (genpts) {
        formatContext->flags |= AVFMT_FLAG_GENPTS;
    }

    av_format_inject_global_side_data(formatContext);

    if (findStreamInfo) {
        AVDictionary **opts = setupFindStreamInfoOpts(formatContext, codecOpts);
        int originNbStreams = formatContext->nb_streams;
        err = avformat_find_stream_info(formatContext, opts);
        if (msgQueue) {
            msgQueue->notifyMsg(Message::MSG_FIND_STREAM_INFO);
        }
        for (i = 0; i < originNbStreams; i++) {
            av_dict_free(&opts[i]);
        }
        av_freep(&opts);
        if (err < 0) {
            ALOGD("%s %s: could not find codec parameters", __func__, videoState->fileName);
            closeReadThread(videoState, formatContext);
            return NEGATIVE(S_NOT_FIND_STREAM_INFO);
        }
    }

    if (formatContext->pb) {
        // FIXME hack, ffplay maybe should not use avio_feof() to test for the end
        formatContext->pb->eof_reached = 0;
    }

    if (seekByBytes < 0) {
        seekByBytes = (formatContext->iformat->flags & AVFMT_TS_DISCONT) &&
                      strcmp(OGG, formatContext->iformat->name) != 0;
    }

    videoState->maxFrameDuration = (formatContext->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;

    if (!windowTitle && (t = av_dict_get(formatContext->metadata, TITLE, nullptr, 0))) {
        windowTitle = av_asprintf("%s - %s", t->value, inputFileName);
    }

    /* if seeking requested, we execute it */
    if (startTime != AV_NOPTS_VALUE) {
        int64_t timestamp = startTime;
        /* add the stream start time */
        if (formatContext->start_time != AV_NOPTS_VALUE) {
            timestamp += formatContext->start_time;
        }
        ret = avformat_seek_file(formatContext, -1, INT64_MIN, timestamp, INT64_MAX, 0);
        if (ret < 0) {
            ALOGD("%s %s: could not seek to position %0.3f", __func__, videoState->fileName,
                  (double) timestamp / AV_TIME_BASE);
        }
    }

    videoState->realTime = isRealTime(formatContext);

    if (showStatus) {
        av_dump_format(formatContext, 0, videoState->fileName, 0);
    }

    for (i = 0; i < formatContext->nb_streams; i++) {
        AVStream *st = formatContext->streams[i];
        enum AVMediaType type = st->codecpar->codec_type;
        st->discard = AVDISCARD_ALL;
        if (type >= 0 && wantedStreamSpec[type] && streamIndex[type] == -1) {
            if (avformat_match_stream_specifier(formatContext, st, wantedStreamSpec[type]) > 0) {
                streamIndex[type] = i;
            }
        }
    }

    for (i = 0; i < AVMEDIA_TYPE_NB; i++) {
        if (wantedStreamSpec[i] && streamIndex[i] == -1) {
            ALOGD("%s Stream specifier %s does not match any stream", __func__, wantedStreamSpec[i]);
            streamIndex[i] = INT_MAX;
        }
    }

    if (!videoDisable) {
        streamIndex[AVMEDIA_TYPE_VIDEO] =
                av_find_best_stream(formatContext,
                                    AVMEDIA_TYPE_VIDEO,
                                    streamIndex[AVMEDIA_TYPE_VIDEO],
                                    -1,
                                    nullptr,
                                    0);
    }

    if (!audioDisable) {
        streamIndex[AVMEDIA_TYPE_AUDIO] =
                av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO,
                                    streamIndex[AVMEDIA_TYPE_AUDIO],
                                    streamIndex[AVMEDIA_TYPE_VIDEO],
                                    nullptr, 0);
    }

    if (!videoDisable && !subtitleDisable) {
        streamIndex[AVMEDIA_TYPE_SUBTITLE] =
                av_find_best_stream(formatContext, AVMEDIA_TYPE_SUBTITLE,
                                    streamIndex[AVMEDIA_TYPE_SUBTITLE],
                                    (streamIndex[AVMEDIA_TYPE_AUDIO] >= 0 ?
                                     streamIndex[AVMEDIA_TYPE_AUDIO] :
                                     streamIndex[AVMEDIA_TYPE_VIDEO]),
                                    nullptr, 0);
    }

    videoState->showMode = showMode;

    if (streamIndex[AVMEDIA_TYPE_VIDEO] >= 0) {
        AVStream *st = formatContext->streams[streamIndex[AVMEDIA_TYPE_VIDEO]];
        AVCodecParameters *codecpar = st->codecpar;
        AVRational sar = av_guess_sample_aspect_ratio(formatContext, st, nullptr);
        if (codecpar->width) {
            // TODO
            // set_default_window_size(codecpar->width, codecpar->height, sar);
        }
    }

    /* open the streams */
    if (streamIndex[AVMEDIA_TYPE_AUDIO] >= 0) {
        streamComponentOpen(streamIndex[AVMEDIA_TYPE_AUDIO]);
    } else {
        videoState->avSyncType = avSyncType = AV_SYNC_VIDEO_MASTER;
    }

    ret = -1;

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
        return NEGATIVE(S_NOT_OPEN_FILE);
    }

    if (infiniteBuffer < 0 && videoState->realTime) {
        infiniteBuffer = 1;
    }

    if (videoState->videoStream && videoState->videoStream->codecpar) {
        AVCodecParameters *codecpar = videoState->videoStream->codecpar;
        if (msgQueue) {
            msgQueue->notifyMsg(Message::MSG_VIDEO_SIZE_CHANGED, codecpar->width, codecpar->height);
            msgQueue->notifyMsg(Message::MSG_SAR_CHANGED, codecpar->width, codecpar->height);
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

    /* offset should be seeked*/
    if (seekAtStart > 0) {
        // ffp_seek_to_l((long) (seekAtStart));
    }

    for (;;) {

        if (videoState->abortRequest) {
            break;
        }

        // network stream pause or resume
        if (videoState->paused != videoState->lastPaused) {
            videoState->lastPaused = videoState->paused;
            if (videoState->paused) {
                videoState->readPauseReturn = av_read_pause(formatContext);
            } else {
                av_read_play(formatContext);
            }
        }

        if (videoState->seekReq) {
            int64_t seek_target = videoState->seekPos;
            int64_t seek_min = videoState->seekRel > 0 ? seek_target - videoState->seekRel + 2 : INT64_MIN;
            int64_t seek_max = videoState->seekRel < 0 ? seek_target - videoState->seekRel - 2 : INT64_MAX;

            // FIXME the +-2 videoState due to rounding being not done in the correct direction in generation
            //      of the seek_pos/seek_rel variables

            ret = avformat_seek_file(videoState->ic, -1, seek_min, seek_target, seek_max, videoState->seekFlags);
            if (ret < 0) {
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
                    videoState->exitClock.setClock(seek_target / (double) AV_TIME_BASE, 0);
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

        /* if the queue are full, no need to read more */
        if (infiniteBuffer < 1 &&
            ((videoState->audioPacketQueue.size +
              videoState->videoPacketQueue.size +
              videoState->subtitlePacketQueue.size) > MAX_QUEUE_SIZE ||
             (streamHasEnoughPackets(videoState->audioStream,
                                     videoState->audioStreamIndex,
                                     &videoState->audioPacketQueue) &&
              streamHasEnoughPackets(videoState->videoStream,
                                     videoState->videoStreamIndex,
                                     &videoState->videoPacketQueue) &&
              streamHasEnoughPackets(videoState->subtitleStream,
                                     videoState->subtitleStreamIndex,
                                     &videoState->subtitlePacketQueue)))) {
            /* wait 10 ms */
            waitMutex->mutexLock();
            videoState->continueReadThread->condWaitTimeout(waitMutex, 10);
            waitMutex->mutexUnLock();
            continue;
        }

        // 未暂停
        bool notPaused = !videoState->paused;
        // 未初始化音频流 或者 解码结束 同时 无可用帧
        bool audioSeekCondition = !videoState->audioStream ||
                                  (videoState->audioDecoder.finished == videoState->audioPacketQueue.serial &&
                                   videoState->audioFrameQueue.frameQueueNbRemaining() == 0);
        // 未初始化视频流 或者 解码结束 同时 无可用帧
        bool videoSeekCondition = !videoState->videoStream ||
                                  (videoState->videoDecoder.finished == videoState->videoPacketQueue.serial &&
                                   videoState->videoFrameQueue.frameQueueNbRemaining() == 0);
        if (notPaused && audioSeekCondition && videoSeekCondition) {
            if (loop != 1 && (!loop || --loop)) {
                streamSeek(startTime != AV_NOPTS_VALUE ? startTime : 0, 0, 0);
            } else if (autoExit) {
                return NEGATIVE(ERROR_EOF);
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

        /* check if packet videoState in play range specified by user, then queue, otherwise discard */
        streamStartTime = formatContext->streams[packet->stream_index]->start_time;
        packetTs = (packet->pts == AV_NOPTS_VALUE) ? packet->dts : packet->pts;
        packetInPlayRange = (duration == AV_NOPTS_VALUE) ||
                            (packetTs - (streamStartTime != AV_NOPTS_VALUE ? streamStartTime : 0)) *
                            av_q2d(formatContext->streams[packet->stream_index]->time_base) -
                            (double) (startTime != AV_NOPTS_VALUE ? startTime : 0) / 1000000 <=
                            ((double) duration / 1000000);

        if (packet->stream_index == videoState->audioStreamIndex && packetInPlayRange) {
            videoState->audioPacketQueue.packetQueuePut(packet);
        } else if (packet->stream_index == videoState->videoStreamIndex && packetInPlayRange &&
                   !(videoState->videoStream->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
            videoState->videoPacketQueue.packetQueuePut(packet);
        } else if (packet->stream_index == videoState->subtitleStreamIndex && packetInPlayRange) {
            videoState->subtitlePacketQueue.packetQueuePut(packet);
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

/* open a given stream. Return 0 if OK */
int FFPlay::streamComponentOpen(int streamIndex) {
    ALOGD("%s streamIndex=%d", __func__, streamIndex);
    return POSITIVE;
}

int FFPlay::streamHasEnoughPackets(AVStream *pStream, int streamIndex, PacketQueue *pQueue) {
    return streamIndex < 0 ||
           pQueue->abortRequest ||
           (pStream->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
           (pQueue->nbPackets > MIN_FRAMES &&
            (!pQueue->duration || av_q2d(pStream->time_base) * pQueue->duration > 1.0));
}

void FFPlay::streamComponentClose(AVStream *pStream) {

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


#pragma clang diagnostic pop
#pragma clang diagnostic pop