#include "decoder/VideoDecoder.h"

VideoDecoder::VideoDecoder(AVFormatContext *formatCtx,
                           AVCodecContext *avctx,
                           AVStream *stream,
                           int streamIndex,
                           PlayerState *playerState,
                           AVPacket *flushPacket,
                           Condition *readWaitCond)
        : MediaDecoder(avctx, stream, streamIndex, playerState, flushPacket, readWaitCond) {
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
    if (decodeThread == nullptr) {
        decodeThread = new Thread(this);
        decodeThread->start();
    }
}

void VideoDecoder::stop() {
    MediaDecoder::stop();
    frameQueue->abort();
    if (decodeThread != nullptr) {
        // https://baike.baidu.com/item/pthread_join
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
    if (DEBUG) {
        ALOGD(TAG, "start video decoder");
    }
    decodeVideo();
    if (DEBUG) {
        ALOGD(TAG, "end video decoder");
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
        if (DEBUG) ALOGE(TAG, "%s not memory", __func__);
        return ERROR_NOT_MEMORY;
    }

    for (;;) {

        ret = popFrame(frame);

        if (ret < 0) {
            if (DEBUG) {
                ALOGE(TAG, "%s not get video frame ret = %d ", __func__, ret);
            }
            break;
        }

        if (ret == 0) {
            if (DEBUG) {
                ALOGD(TAG, "%s drop frame", __func__);
            }
            continue;
        }

        // 计算帧的pts、duration等
        duration = getFrameDuration(frameRate);
        pts = getFramePts(frame, timeBase);

        // 放入到已解码队列
        ret = pushFrame(frame, pts, duration, frame->pkt_pos, packetQueue->getLastSeekSerial());

        // 重置帧
        av_frame_unref(frame);

        if (ret < 0) {
            av_frame_free(&frame);
            if (DEBUG) {
                ALOGE(TAG, "%s not queue picture", __func__);
            }
            break;
        }
    }

    av_frame_free(&frame);
    av_free(frame);
    frame = nullptr;

    return ret;
}

double VideoDecoder::getFramePts(const AVFrame *frame, const AVRational &time_base) const {
    return (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(time_base);
}

double VideoDecoder::getFrameDuration(const AVRational &frame_rate) const {
    return (frame_rate.num && frame_rate.den ? av_q2d((AVRational) {frame_rate.den, frame_rate.num}) : 0);
}

bool VideoDecoder::isFinished() {
    return MediaDecoder::isFinished() && getFrameSize() == 0;
}

int VideoDecoder::popFrame(AVFrame *frame) {
    int ret;

    // 解码视频帧
    if ((ret = decodeFrame(frame)) < 0) {
        if (DEBUG) {
            ALOGE(TAG, "%s video decodeFrame failure ret = %d", __func__, ret);
        }
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
                if (DEBUG) {
                    ALOGD(TAG, "%s video receive frame", __func__);
                }

                if (packetQueue->isAbort()) {
                    if (DEBUG) {
                        ALOGE(TAG, "%s video abort", __func__);
                    }
                    return ERROR_ABORT_REQUEST;
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
                    } else {
                        if (DEBUG)
                            ALOGD(TAG, "%s video receive frame error = %d", __func__, ret);
                        if (ret == AVERROR(EAGAIN)) {
                            if (DEBUG)
                                ALOGD(TAG, "%s video output is not available in this state - user must try to send new input",
                                      __func__);
                        }
                    }
                }

                if (ret == AVERROR_EOF) {
                    finished = packetQueue->getLastSeekSerial();
                    avcodec_flush_buffers(codecContext);
                    return SUCCESS;
                }

                if (ret >= 0) {
                    return SUCCESS;
                }

            } while (ret != AVERROR(EAGAIN));
        }

        do {
            if (DEBUG){
                ALOGD(TAG, "%s video sync packet serial firstSerial = %d lastSerial = %d", __func__,
                      packetQueue->getFirstSeekSerial(),
                      packetQueue->getLastSeekSerial());
            }

            // 同步读取序列
            if (getPacketQueueSize() == 0) {
                readWaitCond->signal();
            }

            if (isPendingPacket) {
                av_packet_move_ref(&packet, &pendingPacket);
                isPendingPacket = false;
            } else {
                // 更新packetSerial
                if (packetQueue->getPacket(&packet) < 0) {
                    return ERROR_ABORT_REQUEST;
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
                if (DEBUG)
                    ALOGD(TAG, "%s video send frame", __func__);
                if (avcodec_send_packet(codecContext, &packet) == AVERROR(EAGAIN)) {
                    if (DEBUG)
                        ALOGE(TAG, "%s video Receive_frame and send_packet both returned EAGAIN, which is an API violation.",
                              __func__);
                    isPendingPacket = true;
                    av_packet_move_ref(&pendingPacket, &packet);
                }
            }
            av_packet_unref(&packet);
        }
    }

    return ret;
}

int VideoDecoder::pushFrame(AVFrame *srcFrame, double pts, double duration, int64_t pos, int serial) {
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
    if (DEBUG) {
        ALOGD(TAG, "%s video frame = %p ptd = %lf duration = %lf pos = %lld serial = %d", __func__, srcFrame, pts, duration,
              pos,
              serial);
    }

    return SUCCESS;
}

int64_t VideoDecoder::getFrameQueueLastPos() {
    return frameQueue->lastPos();
}

