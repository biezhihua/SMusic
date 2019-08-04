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
    return S_ERROR(S_ERROR_UNKNOWN);
}

int FFPlay::shutdown() {
    // TODO
    waitStop();
    return S_ERROR(S_ERROR_UNKNOWN);
}

int FFPlay::waitStop() {
    // TODO
    return S_ERROR(S_ERROR_UNKNOWN);
}


int FFPlay::prepareAsync(const char *fileName) {
    showVersionsAndOptions();

    // TODO Audio Out
//    if (!aOut) {
//        int result = aOut->open();
//        if (!result) {
//            return S_ERROR(SE_NXIO);
//        }
//    }

    videoState = streamOpen(fileName, nullptr);
    if (!videoState) {
        return S_ERROR(SE_NOMEM);
    }
    inputFileName = av_strdup(fileName);
    return S_CORRECT;
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
        if (ret != S_CORRECT) {
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
    return S_ERROR(S_ERROR_EXIT);
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
        play->readThread();
    }
    return 1;
}

VideoState *FFPlay::streamOpen(const char *fileName, AVInputFormat *inputFormat) {

    if (!fileName) {
        ALOGE("%s param fileName is null", __func__);
        return nullptr;
    }

    auto *is = new VideoState();
    if (!is) {
        ALOGD("%s create VideoState OOM", __func__);
        return nullptr;
    }

    if (frameQueueInit(&is->videoFrameQueue, &is->videoPacketQueue, videoQueueSize, 1) < 0) {
        ALOGE("%s video frame queue init fail", __func__);
        streamClose();
        return nullptr;
    }

    if (frameQueueInit(&is->audioFrameQueue, &is->audioPacketQueue, AUDIO_QUEUE_SIZE, 0) < 0) {
        ALOGE("%s audio frame queue init fail", __func__);
        streamClose();
        return nullptr;
    }

    if (frameQueueInit(&is->subtitleFrameQueue, &is->subtitlePacketQueue, SUBTITLE_QUEUE_SIZE, 0) < 0) {
        ALOGE("%s subtitle frame queue init fail", __func__);
        streamClose();
        return nullptr;
    }

    if (packetQueueInit(&is->videoPacketQueue) < 0 ||
        packetQueueInit(&is->audioPacketQueue) < 0 ||
        packetQueueInit(&is->subtitlePacketQueue) < 0) {
        ALOGE("%s packet queue init fail", __func__);
        streamClose();
        return nullptr;
    }

    if (!(is->continueReadThread = new Mutex())) {
        ALOGE("%s create continue read thread mutex fail", __func__);
        streamClose();
        return nullptr;
    }

    if (initClock(&is->videoClock, &is->videoPacketQueue.serial) < 0 ||
        initClock(&is->audioClock, &is->audioPacketQueue.serial) < 0 ||
        initClock(&is->subtitleClock, &is->subtitlePacketQueue.serial) < 0) {
        ALOGE("%s init clock fail", __func__);
        streamClose();
        return nullptr;
    }

    is->fileName = av_strdup(fileName);
    is->audioClockSerial = -1;
    is->inputFormat = inputFormat;
    is->yTop = 0;
    is->xLeft = 0;
    is->audioVolume = getStartupVolume();
    is->muted = 0;
    is->avSyncType = avSyncType;
    is->playMutex = new Mutex();
    is->accurateSeekMutex = new Mutex();
    is->pauseReq = !startOnPrepared;
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

int FFPlay::frameQueueInit(FrameQueue *pFrameQueue, PacketQueue *pPacketQueue, int queueSize, int keepLast) {

    if (!(pFrameQueue->mutex = new Mutex())) {
        ALOGE("%s create mutex fail", __func__);
        return S_ERROR(SE_NOMEM);
    }

    pFrameQueue->packetQueue = pPacketQueue;
    pFrameQueue->maxSize = FFMIN(queueSize, FRAME_QUEUE_SIZE);
    pFrameQueue->keepLast = keepLast;

    for (int i = 0; i < pFrameQueue->maxSize; i++) {
        if (!(pFrameQueue->queue[i].frame = av_frame_alloc())) {
            return S_ERROR(ENOMEM);
        }
    }

    ALOGI("%s packetQueue=%p maxSize=%d keepLast=%d",
          __func__,
          pFrameQueue->packetQueue,
          pFrameQueue->maxSize,
          pFrameQueue->keepLast);

    return S_CORRECT;
}

void FFPlay::streamClose() {

}

int FFPlay::packetQueueInit(PacketQueue *pPacketQueue) {

    if (!pPacketQueue) {
        ALOGE("%s param is null", __func__);
        return S_ERROR(SE_NULL);
    }

    pPacketQueue->mutex = new Mutex();

    if (!pPacketQueue->mutex) {
        ALOGE("%s create mutex fail", __func__);
        return S_ERROR(SE_NOMEM);
    }

    pPacketQueue->abortRequest = 1;

    ALOGI("%s mutex=%p abortRequest=%d",
          __func__,
          pPacketQueue->mutex,
          pPacketQueue->abortRequest);

    return S_CORRECT;
}

int FFPlay::initClock(Clock *pClock, int *pQueueSerial) {
    if (!pClock) {
        return S_ERROR(SE_NULL);
    }
    pClock->speed = 1.0F;
    pClock->paused = 0;
    pClock->queueSerial = pQueueSerial;
    setClock(pClock, NAN, -1);
    return S_CORRECT;
}

void FFPlay::setClock(Clock *pClock, double pts, int serial) {
    double time = av_gettime_relative() / 1000000.0;
    setClockAt(pClock, pts, serial, time);
}

void FFPlay::setClockAt(Clock *pClock, double pts, int serial, double time) {
    if (pClock) {
        pClock->pts = pts;
        pClock->lastUpdated = time;
        pClock->ptsDrift = pClock->pts - time;
        pClock->serial = serial;
    }
}

void FFPlay::readThread() {
    ALOGD(__func__);
}

#pragma clang diagnostic pop