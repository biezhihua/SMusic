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
    ALOGD("%s ===== versions =====", __func__);
    av_log(this, AV_LOG_INFO, "%-*s: %s\n", 13, "FFmpeg", av_version_info());

//    if (!aOut) {
//        int result = aOut->open();
//        if (!result) {
//            return S_ERROR(SE_NXIO);
//        }
//    }
//    videoState = streamOpen(fileName, nullptr);
//    if (!videoState) {
//        return S_ERROR(S_ERROR_UNKNOWN);
//    }
    inputFileName = strdup(fileName);
    return S_CORRECT;
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

