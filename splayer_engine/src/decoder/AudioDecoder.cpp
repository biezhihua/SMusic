#include <decoder/AudioDecoder.h>

AudioDecoder::AudioDecoder(AVFormatContext *formatCtx,
                           AVCodecContext *avctx,
                           AVStream *stream,
                           int streamIndex,
                           PlayerState *playerState,
                           AVPacket *flushPacket,
                           Condition *readWaitCond)
        : MediaDecoder(avctx,
                       stream,
                       streamIndex,
                       playerState,
                       flushPacket,
                       readWaitCond) {
    formatContext = formatCtx;
    frameQueue = new FrameQueue(AUDIO_QUEUE_SIZE, 0, packetQueue);
    decodeThread = nullptr;
}

AudioDecoder::~AudioDecoder() {
    formatContext = nullptr;
    frameQueue->flush();
    delete frameQueue;
    frameQueue = nullptr;
}

void AudioDecoder::start() {
    MediaDecoder::start();
    frameQueue->start();
    if (decodeThread == nullptr) {
        decodeThread = new Thread(this);
        decodeThread->start();
    }
}

void AudioDecoder::stop() {
    MediaDecoder::stop();
    frameQueue->abort();
    if (decodeThread != nullptr) {
        decodeThread->join();
        delete decodeThread;
        decodeThread = nullptr;
    }
}

void AudioDecoder::flush() {
    MediaDecoder::flush();
    frameQueue->flush();
}

int AudioDecoder::getFrameSize() {
    return frameQueue->getFrameSize();
}

FrameQueue *AudioDecoder::getFrameQueue() {
    return frameQueue;
}

void AudioDecoder::run() {
    if (DEBUG) {
        ALOGD(TAG, "start audio decoder");
    }
    decodeAudio();
    if (DEBUG) {
        ALOGD(TAG, "end audio decoder");
    }
}

int AudioDecoder::decodeAudio() {
    AVFrame *frame = av_frame_alloc();
    Frame *af = nullptr;
    AVRational timeBase;
    int ret = 0;

    if (!frame) {
        return ERROR_NOT_MEMORY;
    }

    for (;;) {
        ret = popFrame(frame);

        if (ret < 0) {
            if (DEBUG) {
                ALOGE(TAG, "%s audio not get audio frame ret = %d ", __func__, ret);
            }
            break;
        }

        if (ret == 0) {
            if (DEBUG) {
                ALOGD(TAG, "%s audio drop frame", __func__);
            }
            continue;
        }

        timeBase = (AVRational) {1, frame->sample_rate};

        if ((af = frameQueue->peekWritable()) == nullptr) {
            if (DEBUG) {
                ALOGE(TAG, "%s audio peek writable null", __func__);
            }
            break;
        }

        af->pts = getFramePts(frame, timeBase);
        af->seekSerial = packetQueue->getLastSeekSerial();
        af->pos = frame->pkt_pos;
        af->duration = getFrameDuration(frame);

        av_frame_move_ref(af->frame, frame);
        frameQueue->pushFrame();
    }

    return SUCCESS;
}

double AudioDecoder::getFrameDuration(const AVFrame *avFrame) const {
    return av_q2d((AVRational) {avFrame->nb_samples, avFrame->sample_rate});
}

double AudioDecoder::getFramePts(const AVFrame *avFrame, const AVRational &timeBase) const {
    return (avFrame->pts == AV_NOPTS_VALUE) ? NAN : avFrame->pts * av_q2d(timeBase);
}

int AudioDecoder::popFrame(AVFrame *frame) {
    return decodeFrame(frame);
}

int AudioDecoder::decodeFrame(AVFrame *frame) {

    int ret = AVERROR(EAGAIN);

    for (;;) {
        AVPacket packet;

        if (DEBUG) {
            ALOGD(TAG, "%s audio firstSerial = %d lastSerial = %d", __func__,
                  packetQueue->getFirstSeekSerial(),
                  packetQueue->getLastSeekSerial());
        }

        if (isSamePacketSerial()) {
            // 接收一帧解码后的数据
            do {
                if (DEBUG) {
                    ALOGD(TAG, "%s audio receive frame", __func__);
                }

                if (packetQueue->isAbort()) {
                    if (DEBUG) {
                        ALOGE(TAG, "%s audio abort", __func__);
                    }
                    return ERROR_ABORT_REQUEST;
                }

                if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
                    ret = avcodec_receive_frame(codecContext, frame);
                    if (ret >= 0) {
                        AVRational tb = (AVRational) {1, frame->sample_rate};
                        if (frame->pts != AV_NOPTS_VALUE) {
                            frame->pts = av_rescale_q(frame->pts, codecContext->pkt_timebase, tb);
                        } else if (nextPts != AV_NOPTS_VALUE) {
                            frame->pts = av_rescale_q(nextPts, nextPtsTb, tb);
                        }
                        if (frame->pts != AV_NOPTS_VALUE) {
                            nextPts = frame->pts + frame->nb_samples;
                            nextPtsTb = tb;
                        }
                    } else {
                        if (ret == AVERROR(EAGAIN)) {
                            if (DEBUG) {
                                ALOGD(TAG,
                                      "%s audio output is not available in this state - user must try to send new input",
                                      __func__);
                            }
                        }
                    }
                }

                if (DEBUG) {
                    ALOGD(TAG, "%s audio receive frame ret = %d", __func__, ret);
                }

                if (ret == AVERROR_EOF) {
                    finished = packetQueue->getLastSeekSerial();
                    avcodec_flush_buffers(codecContext);
                    return 0;
                }

                if (ret >= 0) {
                    return SUCCESS;
                }

            } while (ret != AVERROR(EAGAIN));
        }

        if (DEBUG) {
            ALOGD(TAG, "%s audio sync packet serial firstSerial = %d lastSerial = %d", __func__,
                  packetQueue->getFirstSeekSerial(),
                  packetQueue->getLastSeekSerial());
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
                // 更新packetSerial
                if (packetQueue->getPacket(&packet) < 0) {
                    if (DEBUG) {
                        ALOGE(TAG, "%s audio get packet", __func__);
                    }
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
            if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
                if (DEBUG) {
                    ALOGD(TAG, "%s audio send frame", __func__);
                }
                if (avcodec_send_packet(codecContext, &packet) == AVERROR(EAGAIN)) {
                    if (DEBUG) {
                        ALOGE(TAG,
                              "%s audio Receive_frame and send_packet both returned EAGAIN, which is an API violation.",
                              __func__);
                    }
                    isPendingPacket = true;
                    av_packet_move_ref(&pendingPacket, &packet);
                }
            }
            av_packet_unref(&packet);
        }
    }
    return ret;
}

int AudioDecoder::getAudioFrame(Frame **frame) {
    do {
        if ((*frame = frameQueue->peekReadable()) == nullptr) {
            if (DEBUG) {
                ALOGE(TAG, "%s audio peek readable ", __func__);
            }
            return ERROR_AUDIO_PEEK_READABLE;
        }
        frameQueue->popFrame();
    } while ((*frame)->seekSerial != packetQueue->getLastSeekSerial());
    return SUCCESS;
}

bool AudioDecoder::isFinished() {
    return MediaDecoder::isFinished() && getFrameSize() == 0;
}

int64_t AudioDecoder::getFrameQueueLastPos() {
    return frameQueue->lastPos();
}

