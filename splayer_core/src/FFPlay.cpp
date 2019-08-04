
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
    is->fileName = av_strdup(fileName);

    is->inputFormat = inputFormat;
    is->yTop = 0;
    is->xLeft = 0;

    if (frameQueueInit(&is->videoFQueue, &is->videoPQueue, videoQueueSize, 1) < 0) {
        ALOGE("%s video frame queue init fail", __func__);
        streamClose();
        return nullptr;
    }
    if (frameQueueInit(&is->audioFQueue, &is->audioPQueue, AUDIO_QUEUE_SIZE, 0) < 0) {
        ALOGE("%s audio frame queue init fail", __func__);
        streamClose();
        return nullptr;
    }
    if (frameQueueInit(&is->subtitleFQueue, &is->subtitlePQueue, SUBTITLE_QUEUE_SIZE, 0) < 0) {
        ALOGE("%s subtitle frame queue init fail", __func__);
        streamClose();
        return nullptr;
    }

    if (packetQueueInit(&is->videoPQueue) < 0 ||
        packetQueueInit(&is->audioPQueue) < 0 ||
        packetQueueInit(&is->subtitlePQueue) < 0) {

    }


    return is;
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



