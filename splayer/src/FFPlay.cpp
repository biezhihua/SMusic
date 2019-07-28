
#include "../include/FFPlay.h"


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
    return S_FAILURE;
}

int FFPlay::shutdown() {
    // TODO
    waitStop();
    return S_FAILURE;
}

int FFPlay::waitStop() {
    // TODO
    return S_FAILURE;
}

int FFPlay::prepareAsync(const char *fileName) {
    ALOGD("%s fileName=%s", __func__, fileName);

    if (!aOut) {
        int result = aOut->open();
        if (!result) {
            return S_FAILURE;
        }
    }
//    videoState = streamOpen(fileName, nullptr);
//    if (!videoState) {
//        return S_FAILURE;
//    }
    inputFileName = strdup(fileName);
    return 0;
}

int FFPlay::getMsg(Message *msg, bool block) {
    while (true) {
        bool continueWaitNextMsg = false;
        int ret = msgQueue->getMsg(msg, block);
        if (ret == S_FAILURE) {
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
//                if (!mp->ffSPLAYER->start_on_prepared) {
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
    return S_FAILURE;
}

static int decode_interrupt_cb(void *ctx) {
    VideoState *is = static_cast<VideoState *>(ctx);
    return is->abort_request;
}

static int is_realtime(AVFormatContext *s) {
    if (!strcmp(s->iformat->name, "rtp")
        || !strcmp(s->iformat->name, "rtsp")
        || !strcmp(s->iformat->name, "sdp")
            )
        return 1;

    if (s->pb && (!strncmp(s->url, "rtp:", 4)
                  || !strncmp(s->url, "udp:", 4)
    )
            )
        return 1;
    return 0;
}

static int stream_has_enough_packets(AVStream *st, int stream_id, PacketQueue *queue) {
    return stream_id < 0 ||
           queue->abort_request ||
           (st->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
           queue->nb_packets > MIN_FRAMES && (!queue->duration || av_q2d(st->time_base) * queue->duration > 1.0);
}

static int packet_queue_put_private(PacketQueue *q, AVPacket *pkt) {
    MyAVPacketList *pkt1;

    if (q->abort_request)
        return -1;

    pkt1 = static_cast<MyAVPacketList *>(av_malloc(sizeof(MyAVPacketList)));
    if (!pkt1)
        return -1;
    pkt1->pkt = *pkt;
    pkt1->next = NULL;
    if (pkt == &flush_pkt)
        q->serial++;
    pkt1->serial = q->serial;

    if (!q->last_pkt)
        q->first_pkt = pkt1;
    else
        q->last_pkt->next = pkt1;
    q->last_pkt = pkt1;
    q->nb_packets++;
    q->size += pkt1->pkt.size + sizeof(*pkt1);
    q->duration += pkt1->pkt.duration;
    /* XXX: should duplicate packet data in DV case */
    q->mutex->condSignal();
    return 0;
}

static int packet_queue_put(PacketQueue *q, AVPacket *pkt) {
    int ret;

    q->mutex->mutexLock();
    ret = packet_queue_put_private(q, pkt);
    q->mutex->mutexUnLock();

    if (pkt != &flush_pkt && ret < 0)
        av_packet_unref(pkt);

    return ret;
}


static int packet_queue_put_nullpacket(PacketQueue *q, int stream_index) {
    AVPacket pkt1, *pkt = &pkt1;
    av_init_packet(pkt);
    pkt->data = NULL;
    pkt->size = 0;
    pkt->stream_index = stream_index;
    return packet_queue_put(q, pkt);
}


/* this thread gets the stream from the disk or the network */
static int read_thread(void *arg) {
    VideoState *is = static_cast<VideoState *>(arg);
    AVFormatContext *ic = nullptr;
    int err, i, ret;
    int st_index[AVMEDIA_TYPE_NB];
    AVPacket pkt1, *pkt = &pkt1;
    int64_t stream_start_time;
    int pkt_in_play_range = 0;
    AVDictionaryEntry *t;
    Mutex *wait_mutex = new Mutex();
    int scan_all_pmts_set = 0;
    int64_t pkt_ts;

    if (!wait_mutex) {
        av_log(nullptr, AV_LOG_FATAL, "SDL_CreateMutex(): wait_mutex\n");
        ret = AVERROR(ENOMEM);
        return ret;
    }

    memset(st_index, -1, sizeof(st_index));
    is->last_video_stream = is->video_stream = -1;
    is->last_audio_stream = is->audio_stream = -1;
    is->last_subtitle_stream = is->subtitle_stream = -1;
    is->eof = 0;

    ic = avformat_alloc_context();
    if (!ic) {
        av_log(nullptr, AV_LOG_FATAL, "Could not allocate context.\n");
        ret = AVERROR(ENOMEM);
        return ret;
    }
    ic->interrupt_callback.callback = decode_interrupt_cb;
    ic->interrupt_callback.opaque = is;
    if (!av_dict_get(format_opts, "scan_all_pmts", nullptr, AV_DICT_MATCH_CASE)) {
        av_dict_set(&format_opts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
        scan_all_pmts_set = 1;
    }
    err = avformat_open_input(&ic, is->filename, is->iformat, &format_opts);
    if (err < 0) {
        // print_error(is->filename, err);
        ret = -1;
        return ret;
    }
    if (scan_all_pmts_set)
        av_dict_set(&format_opts, "scan_all_pmts", nullptr, AV_DICT_MATCH_CASE);

    if ((t = av_dict_get(format_opts, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
        av_log(nullptr, AV_LOG_ERROR, "Option %s not found.\n", t->key);
        ret = AVERROR_OPTION_NOT_FOUND;
        return ret;
    }
    is->ic = ic;

    // av_format_inject_global_side_data(ic);
//
//    if (find_stream_info) {
//        AVDictionary **opts = setup_find_stream_info_opts(ic, codec_opts);
//        int orig_nb_streams = ic->nb_streams;
//
//        err = avformat_find_stream_info(ic, opts);
//
//        for (i = 0; i < orig_nb_streams; i++)
//            av_dict_free(&opts[i]);
//        av_freep(&opts);
//
//        if (err < 0) {
//            av_log(NULL, AV_LOG_WARNING,
//                   "%s: could not find codec parameters\n", is->filename);
//            ret = -1;
//            goto fail;
//        }
//    }

    if (ic->pb)
        ic->pb->eof_reached = 0; // FIXME hack, ffplay maybe should not use avio_feof() to test for the end

    if (seek_by_bytes < 0)
        seek_by_bytes = (ic->iformat->flags & AVFMT_TS_DISCONT) != 0 && strcmp("ogg", ic->iformat->name);

    is->max_frame_duration = (ic->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;

    if (!window_title && (t = av_dict_get(ic->metadata, "title", NULL, 0)))
//        window_title = av_asprintf("%s - %s", t->value, input_filename);

        /* if seeking requested, we execute it */
        if (start_time != AV_NOPTS_VALUE) {
            int64_t timestamp;

            timestamp = start_time;
            /* add the stream start time */
            if (ic->start_time != AV_NOPTS_VALUE)
                timestamp += ic->start_time;
            ret = avformat_seek_file(ic, -1, INT64_MIN, timestamp, INT64_MAX, 0);
            if (ret < 0) {
                av_log(NULL, AV_LOG_WARNING, "%s: could not seek to position %0.3f\n",
                       is->filename, (double) timestamp / AV_TIME_BASE);
            }
        }

    is->realtime = is_realtime(ic);

    if (show_status)
        av_dump_format(ic, 0, is->filename, 0);

    for (i = 0; i < ic->nb_streams; i++) {
        AVStream *st = ic->streams[i];
        enum AVMediaType type = st->codecpar->codec_type;
        st->discard = AVDISCARD_ALL;
        if (type >= 0 && wanted_stream_spec[type] && st_index[type] == -1)
            if (avformat_match_stream_specifier(ic, st, wanted_stream_spec[type]) > 0)
                st_index[type] = i;
    }
    for (i = 0; i < AVMEDIA_TYPE_NB; i++) {
        if (wanted_stream_spec[i] && st_index[i] == -1) {
//            av_log(NULL, AV_LOG_ERROR, "Stream specifier %s does not match any %s stream\n", wanted_stream_spec[i],
//                   av_get_media_type_string(i));
            st_index[i] = INT_MAX;
        }
    }

    if (!video_disable)
        st_index[AVMEDIA_TYPE_VIDEO] =
                av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO,
                                    st_index[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);
    if (!audio_disable)
        st_index[AVMEDIA_TYPE_AUDIO] =
                av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO,
                                    st_index[AVMEDIA_TYPE_AUDIO],
                                    st_index[AVMEDIA_TYPE_VIDEO],
                                    NULL, 0);
    if (!video_disable && !subtitle_disable)
        st_index[AVMEDIA_TYPE_SUBTITLE] =
                av_find_best_stream(ic, AVMEDIA_TYPE_SUBTITLE,
                                    st_index[AVMEDIA_TYPE_SUBTITLE],
                                    (st_index[AVMEDIA_TYPE_AUDIO] >= 0 ?
                                     st_index[AVMEDIA_TYPE_AUDIO] :
                                     st_index[AVMEDIA_TYPE_VIDEO]),
                                    NULL, 0);

    is->show_mode = show_mode;
    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
        AVStream *st = ic->streams[st_index[AVMEDIA_TYPE_VIDEO]];
        AVCodecParameters *codecpar = st->codecpar;
        AVRational sar = av_guess_sample_aspect_ratio(ic, st, NULL);
//        if (codecpar->width)
//            set_default_window_size(codecpar->width, codecpar->height, sar);
    }

    /* open the streams */
//    if (st_index[AVMEDIA_TYPE_AUDIO] >= 0) {
//        stream_component_open(is, st_index[AVMEDIA_TYPE_AUDIO]);
//    }
//
//    ret = -1;
//    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
//        ret = stream_component_open(is, st_index[AVMEDIA_TYPE_VIDEO]);
//    }
//    if (is->show_mode == SHOW_MODE_NONE)
//        is->show_mode = ret >= 0 ? SHOW_MODE_VIDEO : SHOW_MODE_RDFT;
//
//    if (st_index[AVMEDIA_TYPE_SUBTITLE] >= 0) {
//        stream_component_open(is, st_index[AVMEDIA_TYPE_SUBTITLE]);
//    }

    if (is->video_stream < 0 && is->audio_stream < 0) {
        av_log(nullptr, AV_LOG_FATAL, "Failed to open file '%s' or configure filtergraph\n",
               is->filename);
        ret = -1;
        return ret;
    }

    if (infinite_buffer < 0 && is->realtime)
        infinite_buffer = 1;

    for (;;) {
        if (is->abort_request)
            break;
        if (is->paused != is->last_paused) {
            is->last_paused = is->paused;
            if (is->paused)
                is->read_pause_return = av_read_pause(ic);
            else
                av_read_play(ic);
        }

        if (is->seek_req) {
//            int64_t seek_target = is->seek_pos;
//            int64_t seek_min = is->seek_rel > 0 ? seek_target - is->seek_rel + 2 : INT64_MIN;
//            int64_t seek_max = is->seek_rel < 0 ? seek_target - is->seek_rel - 2 : INT64_MAX;
//// FIXME the +-2 is due to rounding being not done in the correct direction in generation
////      of the seek_pos/seek_rel variables
//
//            ret = avformat_seek_file(is->ic, -1, seek_min, seek_target, seek_max, is->seek_flags);
//            if (ret < 0) {
//                av_log(NULL, AV_LOG_ERROR,
//                       "%s: error while seeking\n", is->ic->url);
//            } else {
//                if (is->audio_stream >= 0) {
//                    packet_queue_flush(&is->audioq);
//                    packet_queue_put(&is->audioq, &flush_pkt);
//                }
//                if (is->subtitle_stream >= 0) {
//                    packet_queue_flush(&is->subtitleq);
//                    packet_queue_put(&is->subtitleq, &flush_pkt);
//                }
//                if (is->video_stream >= 0) {
//                    packet_queue_flush(&is->videoq);
//                    packet_queue_put(&is->videoq, &flush_pkt);
//                }
//                if (is->seek_flags & AVSEEK_FLAG_BYTE) {
//                    set_clock(&is->extclk, NAN, 0);
//                } else {
//                    set_clock(&is->extclk, seek_target / (double) AV_TIME_BASE, 0);
//                }
//            }
//            is->seek_req = 0;
//            is->queue_attachments_req = 1;
//            is->eof = 0;
//            if (is->paused)
//                step_to_next_frame(is);
        }
        if (is->queue_attachments_req) {
//            if (is->video_st && is->video_st->disposition & AV_DISPOSITION_ATTACHED_PIC) {
//                AVPacket copy = {0};
//                if ((ret = av_packet_ref(&copy, &is->video_st->attached_pic)) < 0)
//                    goto fail;
//                packet_queue_put(&is->videoq, &copy);
//                packet_queue_put_nullpacket(&is->videoq, is->video_stream);
//            }
//            is->queue_attachments_req = 0;
        }

        /* if the queue are full, no need to read more */
        if (infinite_buffer < 1 &&
            (is->audioq.size + is->videoq.size + is->subtitleq.size > MAX_QUEUE_SIZE
             || (stream_has_enough_packets(is->audio_st, is->audio_stream, &is->audioq) &&
                 stream_has_enough_packets(is->video_st, is->video_stream, &is->videoq) &&
                 stream_has_enough_packets(is->subtitle_st, is->subtitle_stream, &is->subtitleq)))) {
            /* wait 10 ms */
            wait_mutex->mutexLock();
            wait_mutex->condWaitTimeout(is->continue_read_thread, 10);
            wait_mutex->mutexUnLock();
            continue;
        }
//        if (!is->paused &&
//            (!is->audio_st ||
//             (is->auddec.finished == is->audioq.serial && frame_queue_nb_remaining(&is->sampq) == 0)) &&
//            (!is->video_st ||
//             (is->viddec.finished == is->videoq.serial && frame_queue_nb_remaining(&is->pictq) == 0))) {
//            if (loop != 1 && (!loop || --loop)) {
//                stream_seek(is, start_time != AV_NOPTS_VALUE ? start_time : 0, 0, 0);
//            } else if (autoexit) {
//                ret = AVERROR_EOF;
//                return ret;
//            }
//        }
        ret = av_read_frame(ic, pkt);
        if (ret < 0) {
            if ((ret == AVERROR_EOF || avio_feof(ic->pb)) && !is->eof) {
                if (is->video_stream >= 0)
                    packet_queue_put_nullpacket(&is->videoq, is->video_stream);
                if (is->audio_stream >= 0)
                    packet_queue_put_nullpacket(&is->audioq, is->audio_stream);
                if (is->subtitle_stream >= 0)
                    packet_queue_put_nullpacket(&is->subtitleq, is->subtitle_stream);
                is->eof = 1;
            }
            if (ic->pb && ic->pb->error)
                break;
            wait_mutex->mutexLock();
            wait_mutex->condWaitTimeout(is->continue_read_thread, 10);
            wait_mutex->mutexUnLock();
            continue;
        } else {
            is->eof = 0;
        }
        /* check if packet is in play range specified by user, then queue, otherwise discard */
        stream_start_time = ic->streams[pkt->stream_index]->start_time;
        pkt_ts = pkt->pts == AV_NOPTS_VALUE ? pkt->dts : pkt->pts;
        pkt_in_play_range = duration == AV_NOPTS_VALUE ||
                            (pkt_ts - (stream_start_time != AV_NOPTS_VALUE ? stream_start_time : 0)) *
                            av_q2d(ic->streams[pkt->stream_index]->time_base) -
                            (double) (start_time != AV_NOPTS_VALUE ? start_time : 0) / 1000000
                            <= ((double) duration / 1000000);
        if (pkt->stream_index == is->audio_stream && pkt_in_play_range) {
            packet_queue_put(&is->audioq, pkt);
        } else if (pkt->stream_index == is->video_stream && pkt_in_play_range
                   && !(is->video_st->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
            packet_queue_put(&is->videoq, pkt);
        } else if (pkt->stream_index == is->subtitle_stream && pkt_in_play_range) {
            packet_queue_put(&is->subtitleq, pkt);
        } else {
            av_packet_unref(pkt);
        }
    }
    return 0;
}


VideoState *FFPlay::streamOpen(const char *fileName, AVInputFormat *inputFormat) {
    VideoState *is = new VideoState();
    if (!is) {
        // TODO Fail
        return nullptr;
    }
    is->filename = av_strdup(fileName);
    if (!is->filename) {
        // TODO Fail
        return nullptr;
    }
    is->iformat = inputFormat;
    is->ytop = 0;
    is->xleft = 0;

    /* start video display */
    if (frame_queue_init(&is->pictq, &is->videoq, VIDEO_PICTURE_QUEUE_SIZE, 1) < 0)
        return nullptr;

    if (frame_queue_init(&is->subpq, &is->subtitleq, SUBPICTURE_QUEUE_SIZE, 0) < 0)
        return nullptr;

    if (frame_queue_init(&is->sampq, &is->audioq, SAMPLE_QUEUE_SIZE, 1) < 0)
        return nullptr;

    if (packet_queue_init(&is->videoq) < 0 ||
        packet_queue_init(&is->audioq) < 0 ||
        packet_queue_init(&is->subtitleq) < 0)
        return nullptr;


    if (!(is->continue_read_thread = new Mutex())) {
        av_log(nullptr, AV_LOG_FATAL, "SDL_CreateCond(): read thread\n");
        return nullptr;
    }

    init_clock(&is->vidclk, &is->videoq.serial);
    init_clock(&is->audclk, &is->audioq.serial);
    init_clock(&is->extclk, &is->extclk.serial);

    is->audio_clock_serial = -1;

    if (startup_volume < 0)
        av_log(nullptr, AV_LOG_WARNING, "-volume=%d < 0, setting to 0\n", startup_volume);
    if (startup_volume > 100)
        av_log(nullptr, AV_LOG_WARNING, "-volume=%d > 100, setting to 100\n", startup_volume);

    startup_volume = av_clip(startup_volume, 0, 100);
    startup_volume = av_clip(SDL_MIX_MAXVOLUME * startup_volume / 100, 0, SDL_MIX_MAXVOLUME);
    is->audio_volume = startup_volume;
    is->muted = 0;
    is->av_sync_type = av_sync_type;
    is->read_tid = new Thread(read_thread, is, "read_thread");
    if (!is->read_tid) {
        av_log(nullptr, AV_LOG_FATAL, "SDL_CreateThread(): thread\n");
        return nullptr;
    }
    return is;
}


int FFPlay::frame_queue_init(FrameQueue *f, PacketQueue *pktq, int max_size, int keep_last) {
    int i;
    memset(f, 0, sizeof(FrameQueue));
    if (!(f->mutex = new Mutex())) {
        // av_log(nullptr, AV_LOG_FATAL, "SDL_CreateMutex(): mutex\n");
        return AVERROR(ENOMEM);
    }
    f->pktq = pktq;
    f->max_size = FFMIN(max_size, FRAME_QUEUE_SIZE);
    f->keep_last = keep_last != 0;
//    for (i = 0; i < f->max_size; i++)
//        if (!(f->queue[i].frame = av_frame_alloc()))
//            return AVERROR(ENOMEM);
    return 0;
}

int FFPlay::packet_queue_init(PacketQueue *q) {
    memset(q, 0, sizeof(PacketQueue));
    q->mutex = new Mutex();
    if (!q->mutex) {
        // av_log(nullptr, AV_LOG_FATAL, "SDL_CreateMutex(): mutex\n");
        return AVERROR(ENOMEM);
    }
    q->abort_request = 1;
    return 0;
}

void FFPlay::init_clock(Clock *c, int *queue_serial) {
    c->speed = 1.0;
    c->paused = 0;
    c->queue_serial = queue_serial;
    set_clock(c, NAN, -1);
}

void FFPlay::set_clock(Clock *c, double pts, int serial) {
//    double time = av_gettime_relative() / 1000000.0;
//    set_clock_at(c, pts, serial, time);
}

void FFPlay::set_clock_at(Clock *c, double pts, int serial, double time) {

}
