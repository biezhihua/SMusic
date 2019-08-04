
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
        if (ret != POSITIVE) {
            return ret;
        }

        switch (msg->what) {
            case Message::MSG_PREPARED:
                ALOGD("ijkmp_get_msg: MSG_PREPARED\n");
//                pthread_mutex_lock(&mp->mutex);
//                if (mp->mp_state == State::STATE_ASYNC_PREPARING) {
//                    ijkmp_change_state_l(mp, STATE_PREPARED);
//                } else {
//                    // FIXME: 1: onError() ?
//                    av_log(mp->ffSPLAYER, AV_LOG_DEBUG, "MSG_PREPARED: expecting mp_state==STATE_ASYNC_PREPARING\n");
//                }
//                if (!mp->ffSPLAYER->startOnPrepared1) {
//                    ijkmp_change_state_l(mp, STATE_PAUSED);
//                }
//                pthread_mutex_unlock(&mp->mutex);
                break;

            case Message::MSG_COMPLETED:
                ALOGD("ijkmp_get_msg: MSG_COMPLETED\n");
//
//                pthread_mutex_lock(&mp->mutex);
//                mp->restart = 1;
//                mp->restart_from_beginning = 1;
//                ijkmp_change_state_l(mp, STATE_COMPLETED);
//                pthread_mutex_unlock(&mp->mutex);
                break;

            case Message::MSG_SEEK_COMPLETE:
                ALOGD("ijkmp_get_msg: MSG_SEEK_COMPLETE\n");
//
//                pthread_mutex_lock(&mp->mutex);
//                mp->seek_req = 0;
//                mp->seek_msec = 0;
//                pthread_mutex_unlock(&mp->mutex);
                break;

            case Message::REQ_START:
                ALOGD("ijkmp_get_msg: REQ_START\n");
//                continueWaitNextMsg = 1;
//                pthread_mutex_lock(&mp->mutex);
//                if (0 == ikjmp_chkst_start_l(mp->mp_state)) {
//                    // FIXME: 8 check seekable
//                    if (mp->restart) {
//                        if (mp->restart_from_beginning) {
//                            av_log(mp->ffSPLAYER, AV_LOG_DEBUG, "ijkmp_get_msg: REQ_START: restart from beginning\n");
//                            ret = ffp_start_from_l(mp->ffSPLAYER, 0);
//                            if (ret == 0)
//                                ijkmp_change_state_l(mp, STATE_STARTED);
//                        } else {
//                            av_log(mp->ffSPLAYER, AV_LOG_DEBUG, "ijkmp_get_msg: REQ_START: restart from seek pos\n");
//                            ret = ffp_start_l(mp->ffSPLAYER);
//                            if (ret == 0)
//                                ijkmp_change_state_l(mp, STATE_STARTED);
//                        }
//                        mp->restart = 0;
//                        mp->restart_from_beginning = 0;
//                    } else {
//                        av_log(mp->ffSPLAYER, AV_LOG_DEBUG, "ijkmp_get_msg: REQ_START: start on fly\n");
//                        ret = ffp_start_l(mp->ffSPLAYER);
//                        if (ret == 0)
//                            ijkmp_change_state_l(mp, STATE_STARTED);
//                    }
//                }
//                pthread_mutex_unlock(&mp->mutex);
                break;

            case Message::REQ_PAUSE:
                ALOGD("ijkmp_get_msg: REQ_PAUSE\n");
//                continueWaitNextMsg = 1;
//                pthread_mutex_lock(&mp->mutex);
//                if (0 == ikjmp_chkst_pause_l(mp->mp_state)) {
//                    int pause_ret = ffp_pause_l(mp->ffSPLAYER);
//                    if (pause_ret == 0)
//                        ijkmp_change_state_l(mp, STATE_PAUSED);
//                }
//                pthread_mutex_unlock(&mp->mutex);
                break;

            case Message::REQ_SEEK:
                ALOGD("ijkmp_get_msg: REQ_SEEK\n");
//                continueWaitNextMsg = 1;
//
//                pthread_mutex_lock(&mp->mutex);
//                if (0 == ikjmp_chkst_seek_l(mp->mp_state)) {
//                    mp->restart_from_beginning = 0;
//                    if (0 == ffp_seek_to_l(mp->ffSPLAYER, msg->arg1)) {
//                        av_log(mp->ffSPLAYER, AV_LOG_DEBUG, "ijkmp_get_msg: REQ_SEEK: seek to %d\n", (int) msg->arg1);
//                    }
//                }
//                pthread_mutex_unlock(&mp->mutex);
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
    return NEGATIVE(ERROR_EXIT);
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
    VideoState *is = videoState;
    if (!is) {
        return NEGATIVE(S_NULL);
    }
    AVFormatContext *ic = nullptr;
    int err, i, ret;
    int st_index[AVMEDIA_TYPE_NB];
    AVPacket pkt1, *pkt = &pkt1;
    int64_t streamStartTime;
    int pktInPlayRange = 0;
    AVDictionaryEntry *t;
    auto *waitMutex = new Mutex();
    int scan_all_pmts_set = 0;
    int64_t pktTs;

    if (!waitMutex) {
        ALOGE("%s create wait mutex fail", __func__);
        // TODO
        return NEGATIVE(S_NOT_MEMORY);
    }

    memset(st_index, -1, sizeof(st_index));

    is->lastVideoStream = is->videoStreamIndex = -1;
    is->lastAudioStream = is->audioStreamIndex = -1;
    is->lastSubtitleStream = is->subtitleStreamIndex = -1;
    is->eof = 0;

    ic = avformat_alloc_context();
    if (!ic) {
        ALOGE("%s avformat could not allocate context", __func__);
        // TODO
        return NEGATIVE(S_NOT_MEMORY);
    }

    ic->interrupt_callback.callback = decodeInterruptCallback;
    ic->interrupt_callback.opaque = is;

    if (!av_dict_get(formatOpts, "scan_all_pmts", nullptr, AV_DICT_MATCH_CASE)) {
        av_dict_set(&formatOpts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
        scan_all_pmts_set = 1;
    }

    if (av_stristart(is->fileName, "rtmp", nullptr) ||
        av_stristart(is->fileName, "rtsp", nullptr)) {
        // TODO
        av_dict_set(&formatOpts, "timeout", nullptr, 0);
    }

    if (skipCalcFrameRate) {
        av_dict_set_int(&ic->metadata, "skip-calc-frame-rate", skipCalcFrameRate, 0);
        av_dict_set_int(&formatOpts, "skip-calc-frame-rate", skipCalcFrameRate, 0);
    }

    if (inputFormatName) {
        is->inputFormat = av_find_input_format(inputFormatName);
    }

    err = avformat_open_input(&ic, is->fileName, is->inputFormat, &formatOpts);
    if (err < 0) {
        ALOGE("%s avformat could not open input", __func__);
        return NEGATIVE(S_NOT_OPEN_INPUT);
    }

    if (msgQueue) {
        msgQueue->notifyMsg(Message::MSG_OPEN_INPUT);
    }

    if (scan_all_pmts_set) {
        av_dict_set(&formatOpts, "scan_all_pmts", nullptr, AV_DICT_MATCH_CASE);
    }

    if ((t = av_dict_get(formatOpts, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
        ALOGE("%s option %s not found.", __func__, t->key);
        return NEGATIVE(ERROR_OPTION_NOT_FOUND);
    }
    is->ic = ic;

    if (genpts) {
        ic->flags |= AVFMT_FLAG_GENPTS;
    }

    av_format_inject_global_side_data(ic);

    if (findStreamInfo) {
        AVDictionary **opts = setup_find_stream_info_opts(ic, codecOpts);
        int orig_nb_streams = ic->nb_streams;
        if (av_stristart(is->fileName, "data:", nullptr) && orig_nb_streams > 0) {
            for (i = 0; i < orig_nb_streams; i++) {
                if (!ic->streams[i] || !ic->streams[i]->codecpar ||
                    ic->streams[i]->codecpar->profile == FF_PROFILE_UNKNOWN) {
                    break;
                }
            }
        }
        err = avformat_find_stream_info(ic, opts);
        if (msgQueue) {
            msgQueue->notifyMsg(Message::MSG_FIND_STREAM_INFO);
        }
        for (i = 0; i < orig_nb_streams; i++) {
            av_dict_free(&opts[i]);
        }
        av_freep(&opts);
        if (err < 0) {
            ALOGD("%s %s: could not find codec parameters", __func__, is->fileName);
            return NEGATIVE(S_NOT_FIND_PARAMETERS);
        }
    }

    if (ic->pb) {
        ic->pb->eof_reached = 0; // FIXME hack, ffplay maybe should not use avio_feof() to test for the end
    }

    if (seekByBytes < 0) {
        seekByBytes = (ic->iformat->flags & AVFMT_TS_DISCONT) && strcmp("ogg", ic->iformat->name) != 0;
    }

    is->maxFrameDuration = (ic->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;

    if (!windowTitle && (t = av_dict_get(ic->metadata, "title", nullptr, 0))) {
        windowTitle = av_asprintf("%s - %s", t->value, inputFileName);
    }

    /* if seeking requested, we execute it */
    if (startTime != AV_NOPTS_VALUE) {
        int64_t timestamp = startTime;
        /* add the stream start time */
        if (ic->start_time != AV_NOPTS_VALUE) {
            timestamp += ic->start_time;
        }
        ret = avformat_seek_file(ic, -1, INT64_MIN, timestamp, INT64_MAX, 0);
        if (ret < 0) {
            ALOGD("%s %s: could not seek to position %0.3f", __func__, is->fileName, (double) timestamp / AV_TIME_BASE);
        }
    }

    is->realTime = isRealTime(ic);

    av_dump_format(ic, 0, is->fileName, 0);

    for (i = 0; i < ic->nb_streams; i++) {
        AVStream *st = ic->streams[i];
        enum AVMediaType type = st->codecpar->codec_type;
        st->discard = AVDISCARD_ALL;
        if (type >= 0 && wantedStreamSpec[type] && st_index[type] == -1) {
            if (avformat_match_stream_specifier(ic, st, wantedStreamSpec[type]) > 0) {
                st_index[type] = i;
            }
        }
    }

    int video_stream_count = 0;
    int h264_stream_count = 0;
    int first_h264_stream = -1;
    for (i = 0; i < ic->nb_streams; i++) {
        AVStream *st = ic->streams[i];
        enum AVMediaType type = st->codecpar->codec_type;
        if (type == AVMEDIA_TYPE_VIDEO) {
            enum AVCodecID codec_id = st->codecpar->codec_id;
            video_stream_count++;
            if (codec_id == AV_CODEC_ID_H264) {
                h264_stream_count++;
                if (first_h264_stream < 0) {
                    first_h264_stream = i;
                }
            }
        }
    }

    if (video_stream_count > 1 && st_index[AVMEDIA_TYPE_VIDEO] < 0) {
        st_index[AVMEDIA_TYPE_VIDEO] = first_h264_stream;
        ALOGD("%s multiple video stream found, prefer first h264 stream: %d", __func__, first_h264_stream);
    }

    for (i = 0; i < AVMEDIA_TYPE_NB; i++) {
        if (wantedStreamSpec[i] && st_index[i] == -1) {
            ALOGD("%s Stream specifier %s does not match any stream", __func__, wantedStreamSpec[i]);
            st_index[i] = INT_MAX;
        }
    }

    if (!videoDisable) {
        st_index[AVMEDIA_TYPE_VIDEO] =
                av_find_best_stream(ic,
                                    AVMEDIA_TYPE_VIDEO,
                                    st_index[AVMEDIA_TYPE_VIDEO],
                                    -1,
                                    nullptr,
                                    0);
    }
    if (!audioDisable) {
        st_index[AVMEDIA_TYPE_AUDIO] =
                av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO,
                                    st_index[AVMEDIA_TYPE_AUDIO],
                                    st_index[AVMEDIA_TYPE_VIDEO],
                                    nullptr, 0);
    }
    if (!videoDisable && !subtitleDisable) {
        st_index[AVMEDIA_TYPE_SUBTITLE] =
                av_find_best_stream(ic, AVMEDIA_TYPE_SUBTITLE,
                                    st_index[AVMEDIA_TYPE_SUBTITLE],
                                    (st_index[AVMEDIA_TYPE_AUDIO] >= 0 ?
                                     st_index[AVMEDIA_TYPE_AUDIO] :
                                     st_index[AVMEDIA_TYPE_VIDEO]),
                                    nullptr, 0);
    }

    is->showMode = showMode;

    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
        AVStream *st = ic->streams[st_index[AVMEDIA_TYPE_VIDEO]];
        AVCodecParameters *codecpar = st->codecpar;
        AVRational sar = av_guess_sample_aspect_ratio(ic, st, nullptr);
        if (codecpar->width) {
            // TODO
            // set_default_window_size(codecpar->width, codecpar->height, sar);
        }
    }

    /* open the streams */
    if (st_index[AVMEDIA_TYPE_AUDIO] >= 0) {
        streamComponentOpen(st_index[AVMEDIA_TYPE_AUDIO]);
    } else {
        is->avSyncType = avSyncType = AV_SYNC_VIDEO_MASTER;
    }

    ret = -1;
    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
        ret = streamComponentOpen(st_index[AVMEDIA_TYPE_VIDEO]);
    }
    if (is->showMode == SHOW_MODE_NONE) {
        is->showMode = ret >= 0 ? SHOW_MODE_VIDEO : SHOW_MODE_RDFT;
    }

    if (st_index[AVMEDIA_TYPE_SUBTITLE] >= 0) {
        streamComponentOpen(st_index[AVMEDIA_TYPE_SUBTITLE]);
    }

    if (msgQueue) {
        msgQueue->notifyMsg(Message::MSG_COMPONENT_OPEN);
    }

    if (is->videoStreamIndex < 0 && is->audioStreamIndex < 0) {
        ALOGD("%s Failed to open file '%s' or configure filtergraph", __func__, is->fileName);
        return NEGATIVE(S_NOT_OPEN_FILE);
    }

    if (infiniteBuffer < 0 && is->realTime) {
        infiniteBuffer = 1;
    }

    if (!renderWaitStart && !startOnPrepared) {
        // TODO
        // toggle_pause(1);
    }

    if (is->videoStream && is->videoStream->codecpar) {
        AVCodecParameters *codecpar = is->videoStream->codecpar;
        if (msgQueue) {
            msgQueue->notifyMsg(Message::MSG_VIDEO_SIZE_CHANGED, codecpar->width, codecpar->height);
            msgQueue->notifyMsg(Message::MSG_SAR_CHANGED, codecpar->width, codecpar->height);
        }
    }

    prepared = true;

    if (msgQueue) {
        msgQueue->notifyMsg(Message::MSG_PREPARED);
    }

    if (!renderWaitStart && !startOnPrepared) {
        while (is->pauseReq && !is->abortRequest) {
            // SDL_Delay(20);
            // Delay
            // TODO
        }
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

        if (is->abortRequest) {
            break;
        }

        if (is->paused != is->lastPaused) {
            is->lastPaused = is->paused;
            if (is->paused) {
                is->readPauseReturn = av_read_pause(ic);
            } else {
                av_read_play(ic);
            }
        }

        if (is->queueAttachmentsReq) {
            if (is->videoStream && is->videoStream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
                AVPacket copy = {nullptr};
                if ((av_packet_ref(&copy, &is->videoStream->attached_pic)) < 0) {
                    return NEGATIVE(S_NOT_ATTACHED_PIC);
                }
                is->videoPacketQueue.packetQueuePut(&copy);
                is->videoPacketQueue.packetQueuePutNullPacket(is->videoStreamIndex);
            }
            is->queueAttachmentsReq = 0;
        }

        /* if the queue are full, no need to read more */
        if (infiniteBuffer < 1 &&
            (is->audioPacketQueue.size + is->videoPacketQueue.size + is->subtitlePacketQueue.size > MAX_QUEUE_SIZE ||
             (streamHasEnoughPackets(is->audioStream, is->audioStreamIndex, &is->audioPacketQueue) &&
              streamHasEnoughPackets(is->videoStream, is->videoStreamIndex, &is->videoPacketQueue) &&
              streamHasEnoughPackets(is->subtitleStream, is->subtitleStreamIndex, &is->subtitlePacketQueue)))) {
            /* wait 10 ms */
            waitMutex->mutexLock();
            is->continueReadThread->condWaitTimeout(waitMutex, 10);
            waitMutex->mutexUnLock();
            continue;
        }

        if (!is->paused &&
            (!is->audioStream || (is->audioDecoder.finished == is->audioPacketQueue.serial &&
                                  is->audioFrameQueue.frameQueueNbRemaining() == 0)) &&
            (!is->videoStream || (is->videoDecoder.finished == is->videoPacketQueue.serial &&
                                  is->videoFrameQueue.frameQueueNbRemaining() == 0))) {
            if (loop != 1 && (!loop || --loop)) {
                // stream_seek(is, startTime != AV_NOPTS_VALUE ? startTime : 0, 0, 0);
                // TODO
            } else if (autoExit) {
                return NEGATIVE(ERROR_EOF);
            }
        }

        ret = av_read_frame(ic, pkt);

        if (ret < 0) {
            if ((ret == AVERROR_EOF || avio_feof(ic->pb)) && !is->eof) {
                if (is->videoStreamIndex >= 0) {
                    is->videoPacketQueue.packetQueuePutNullPacket(is->videoStreamIndex);
                }
                if (is->audioStreamIndex >= 0) {
                    is->audioPacketQueue.packetQueuePutNullPacket(is->audioStreamIndex);
                }
                if (is->subtitleStreamIndex >= 0) {
                    is->subtitlePacketQueue.packetQueuePutNullPacket(is->subtitleStreamIndex);
                }
                is->eof = 1;
            }
            if (ic->pb && ic->pb->error) {
                break;
            }
            waitMutex->mutexLock();
            is->continueReadThread->condWaitTimeout(waitMutex, 10);
            waitMutex->mutexUnLock();
            continue;
        } else {
            is->eof = 0;
        }

        /* check if packet is in play range specified by user, then queue, otherwise discard */
        streamStartTime = ic->streams[pkt->stream_index]->start_time;
        pktTs = (pkt->pts == AV_NOPTS_VALUE) ? pkt->dts : pkt->pts;
        pktInPlayRange = (duration == AV_NOPTS_VALUE) ||
                         (pktTs - (streamStartTime != AV_NOPTS_VALUE ? streamStartTime : 0)) *
                         av_q2d(ic->streams[pkt->stream_index]->time_base) -
                         (double) (startTime != AV_NOPTS_VALUE ? startTime : 0) / 1000000
                         <= ((double) duration / 1000000);

        if (pkt->stream_index == is->audioStreamIndex && pktInPlayRange) {
            is->audioPacketQueue.packetQueuePut(pkt);
        } else if (pkt->stream_index == is->videoStreamIndex && pktInPlayRange
                   && !(is->videoStream->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
            is->videoPacketQueue.packetQueuePut(pkt);
        } else if (pkt->stream_index == is->subtitleStreamIndex && pktInPlayRange) {
            is->subtitlePacketQueue.packetQueuePut(pkt);
        } else {
            av_packet_unref(pkt);
        }
    }

    return POSITIVE;
}

int FFPlay::isRealTime(AVFormatContext *s) {
    if (!strcmp(s->iformat->name, "rtp") || !strcmp(s->iformat->name, "rtsp") || !strcmp(s->iformat->name, "sdp")) {
        return POSITIVE;
    }
    if (s->pb && (!strncmp(s->url, "rtp:", 4) || !strncmp(s->url, "udp:", 4))) {
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

#pragma clang diagnostic pop