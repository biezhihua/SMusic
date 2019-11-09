#include "VideoDecoder.h"

VideoDecoder::VideoDecoder(AVFormatContext *formatCtx,
                           AVCodecContext *avctx,
                           AVStream *stream,
                           int streamIndex,
                           PlayerInfoStatus *playerState,
                           AVPacket *flushPacket,
                           Condition *readWaitCond, AVDictionary *opts,
                           MessageCenter *messageCenter)
        : MediaDecoder(avctx, stream, streamIndex, playerState, flushPacket, readWaitCond, opts,
                       messageCenter) {
    formatContext = formatCtx;
    frameQueue = new FrameQueue(VIDEO_QUEUE_SIZE, 1, packetQueue);
    decodeThread = nullptr;
    masterClock = nullptr;
    // 旋转角度
    AVDictionaryEntry *entry = av_dict_get(stream->metadata, "rotate", nullptr, AV_DICT_MATCH_CASE);
    if (entry && entry->value) {
        rotate = atoi(entry->value);
    } else {
        rotate = 0;
    }
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s] rotate=%d", __func__, rotate);
    }
}

VideoDecoder::~VideoDecoder() {
    formatContext = nullptr;
    frameQueue->flush();
    delete frameQueue;
    frameQueue = nullptr;
    masterClock = nullptr;
}

void VideoDecoder::setMasterClock(MediaClock *masterClock) {
    this->masterClock = masterClock;
}

void VideoDecoder::start() {
    MediaDecoder::start();
    frameQueue->start();
    if (!decodeThread) {
        decodeThread = new Thread(this);
        decodeThread->start();
    }
}

void VideoDecoder::stop() {
    if (ENGINE_DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }
    mutex.lock();
    MediaDecoder::stop();
    frameQueue->abort();
    mutex.unlock();
    if (decodeThread) {
        decodeThread->join();
        delete decodeThread;
        decodeThread = nullptr;
    }
}

void VideoDecoder::flush() {
    MediaDecoder::flush();
    frameQueue->flush();
}

int VideoDecoder::getFrameSize() {
    return frameQueue->getFrameSize();
}

int VideoDecoder::getRotate() {
    return rotate;
}

FrameQueue *VideoDecoder::getFrameQueue() {
    return frameQueue;
}

void VideoDecoder::run() {
    int ret = 0;
    if ((ret = decodeVideo()) < 0) {
        if (ret != ERROR_FRAME_QUEUE_NOT_WRITABLE) {
            ALOGE(TAG, "[%s] decodeVideo ret = %d ", __func__, ret);
            notifyMsg(Msg::MSG_CHANGE_STATUS, ERRORED);
            notifyMsg(Msg::MSG_STATUS_ERRORED);
            notifyMsg(Msg::MSG_ERROR, ret);
        }
    }
}

/**
 * 解码视频数据包并放入帧队列
 */
int VideoDecoder::decodeVideo() {
    AVFrame *frame = av_frame_alloc();
    int ret = 0;
    double pts;
    double duration;

    AVRational timeBase = stream->time_base;
    AVRational frameRate = av_guess_frame_rate(formatContext, stream, nullptr);

    if (!frame) {
        ALOGE(TAG, "[%s] not memory", __func__);
        return ERROR_NOT_MEMORY;
    }

    for (;;) {

        ret = decodeFrameFromPacketQueue(frame);

        if (ret < 0) {
            ALOGE(TAG, "[%s] not get video frame ret = %d ", __func__, ret);
            break;
        }

        if (ret == 0) {
            if (ENGINE_DEBUG) {
                ALOGD(TAG, "[%s] drop frame", __func__);
            }
            continue;
        }

        // 计算帧的pts、duration等
        duration =
                frameRate.num && frameRate.den ? av_q2d((AVRational) {frameRate.den, frameRate.num})
                                               : 0;
        pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(timeBase);

        // 放入到已解码队列
        ret = pushFrame(frame, pts, duration, frame->pkt_pos, packetQueue->getFirstSeekSerial());

        // 重置帧
        av_frame_unref(frame);

        if (ret < 0) {
            if (ENGINE_DEBUG) {
                ALOGD(TAG, "[%s] not queue picture", __func__);
            }
            break;
        }
    }

    av_frame_free(&frame);
    av_free(frame);
    frame = nullptr;

    return ret;
}

bool VideoDecoder::isFinished() {
    return MediaDecoder::isFinished() && getFrameSize() == 0;
}

int VideoDecoder::decodeFrameFromPacketQueue(AVFrame *frame) {
    int ret;

    // 解码视频帧
    if ((ret = decodeFrame(frame)) < 0) {
        ALOGE(TAG, "[%s] video decodeFrame failure ret = %d", __func__, ret);
        return ERROR_VIDEO_DECODE_FRAME;
    }

    // 判断是否解码成功
    if (ret) {
        double dpts = NAN;

        if (frame->pts != AV_NOPTS_VALUE) {
            dpts = av_q2d(stream->time_base) * frame->pts;
        }

        frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(formatContext, stream, frame);

        // 判断是否需要舍弃该帧
        if (playerState->dropFrameWhenSlow > 0 ||
            (playerState->dropFrameWhenSlow && playerState->syncType != AV_SYNC_VIDEO)) {
            if (frame->pts != AV_NOPTS_VALUE) {
                // diff > 0 当前帧显示时间略超于出主时钟
                // diff < 0 当前帧显示时间慢于主时钟
                double diff = dpts - masterClock->getClock();

                if (!isnan(diff) && // isNoNan
                    fabs(diff) < AV_NOSYNC_THRESHOLD && // isNoSync
                    diff < 0 && // isNeedCorrection
                    isSamePacketSerial() && // isSameSerial
                    getPacketQueueSize() > 0 // isLegalSize
                        ) {
                    av_frame_unref(frame);
                    ret = 0;
                }
            }
        }
    }

    return ret;
}

int VideoDecoder::decodeFrame(AVFrame *frame) {
    int ret = AVERROR(EAGAIN);
    for (;;) {
        AVPacket packet;

        if (isSamePacketSerial()) {
            // 接收一帧解码后的数据
            do {
                if (packetQueue->isAbort()) {
                    if (ENGINE_DEBUG) {
                        ALOGD(TAG, "[%s] video abort", __func__);
                    }
                    return EXIT;
                }

                if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
                    ret = avcodec_receive_frame(codecContext, frame);
                    if (ret >= 0) {
                        // 是否使用通过解码器估算过的时间
                        if (playerState->decoderReorderPts == -1) {
                            // frame timestamp estimated using various heuristics, in stream time base
                            frame->pts = frame->best_effort_timestamp;
                        } else if (!playerState->decoderReorderPts) {
                            // This is also the Presentation time of this AVFrame calculated from
                            // only AVPacket.dts values without pts values.
                            frame->pts = frame->pkt_dts;
                        }
                    }
                }

                if (ret == AVERROR_EOF) {
                    finished = packetQueue->getFirstSeekSerial();
                    avcodec_flush_buffers(codecContext);
                    return SUCCESS;
                }

                if (ret >= 0) {
                    return SUCCESS;
                }

            } while (ret != AVERROR(EAGAIN));
        }

        do {
            // 同步读取序列
            if (getPacketQueueSize() == 0) {
                readWaitCond->signal();
            }

            if (isPendingPacket) {
                av_packet_move_ref(&packet, &pendingPacket);
                isPendingPacket = false;
            } else {
                if (packetQueue->getPacket(&packet) < 0) {
                    ALOGE(TAG, "[%s] video get packet", __func__);
                    return EXIT;
                }
            }
        } while (!isSamePacketSerial());

        if (packet.data == flushPacket->data) {
            avcodec_flush_buffers(codecContext);
            finished = 0;
            nextPts = startPts;
            nextPtsTb = startPtsTb;
        } else {
            if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
                if (avcodec_send_packet(codecContext, &packet) == AVERROR(EAGAIN)) {
                    ALOGE(TAG,
                          "[%s] video Receive_frame and send_packet both returned EAGAIN, which is an API violation.",
                          __func__);
                    isPendingPacket = true;
                    av_packet_move_ref(&pendingPacket, &packet);
                }
            }
            av_packet_unref(&packet);
        }
    }
}

int
VideoDecoder::pushFrame(AVFrame *srcFrame, double pts, double duration, int64_t pos, int serial) {
    Frame *frame;

    if (!(frame = frameQueue->peekWritable())) {
        return ERROR_FRAME_QUEUE_NOT_WRITABLE;
    }

    frame->sampleAspectRatio = srcFrame->sample_aspect_ratio;
    frame->uploaded = 0;

    frame->width = srcFrame->width;
    frame->height = srcFrame->height;
    frame->format = srcFrame->format;

    frame->pts = pts;
    frame->duration = duration;
    frame->pos = pos;
    frame->seekSerial = serial;

    av_frame_move_ref(frame->frame, srcFrame);

    frameQueue->pushFrame();

//    if (ENGINE_DEBUG) {
//        ALOGD(TAG, "[%s] video frame = %p ptd = %lf duration = %lf pos = %lld serial = %d",
//              __func__,
//              srcFrame, pts, duration, pos, serial);
//    }

    return SUCCESS;
}

int64_t VideoDecoder::getFrameQueueLastPos() {
    return frameQueue->currentPos();
}

