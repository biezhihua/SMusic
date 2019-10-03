#include <decoder/AudioDecoder.h>

AudioDecoder::AudioDecoder(AVFormatContext *formatCtx,
                           AVCodecContext *avctx,
                           AVStream *stream,
                           int streamIndex,
                           PlayerState *playerState,
                           AVPacket *flushPacket,
                           Condition *readWaitCond, AVDictionary *opts, MessageCenter *messageCenter)
        : MediaDecoder(avctx,
                       stream,
                       streamIndex,
                       playerState,
                       flushPacket,
                       readWaitCond, opts, messageCenter) {
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
    if (!decodeThread) {
        decodeThread = new Thread(this);
        decodeThread->start();
    }
}

void AudioDecoder::stop() {
    MediaDecoder::stop();
    frameQueue->abort();
    if (decodeThread) {
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
        ALOGD(TAG, "audio decoder - start");
    }
    int ret = 0;
    if ((ret = decodeAudio()) < 0) {
        notifyMsg(Msg::MSG_ERROR, ret);
        notifyMsg(Msg::MSG_REQUEST_ERROR, Msg::MSG_REQUEST_STOP);
    }
    if (DEBUG) {
        ALOGD(TAG, "audio decoder - end");
    }
}

int AudioDecoder::decodeAudio() {

    AVFrame *avFrame = av_frame_alloc();
    Frame *frame = nullptr;
    AVRational timeBase;
    int ret = 0;

    if (!avFrame) {
        if (DEBUG) {
            ALOGE(TAG, "%s not memory", __func__);
        }
        return ERROR_NOT_MEMORY;
    }

    for (;;) {

        if (DEBUG) {
            ALOGD(TAG, "%s start audio decode frame "
                       "firstSeekSerial = %d "
                       "lastSeekSerial = %d ",
                  __func__,
                  packetQueue->getFirstSeekSerial(), packetQueue->getLastSeekSerial());
        }

        // 从PacketQueue队列中获取一个被解码过的Packet
        ret = decodeFrameFromPacketQueue(avFrame);

        if (ret < 0) {
            if (DEBUG) {
                ALOGE(TAG, "%s audio not get audio srcFrame ret = %d ", __func__, ret);
            }
            break;
        }

        if (ret == 0) {
            if (DEBUG) {
                ALOGW(TAG, "%s audio drop srcFrame", __func__);
            }
            continue;
        }

        // 从队列中获取一个可写的Frame对象
        if (!(frame = frameQueue->peekWritable())) {
            if (DEBUG) {
                ALOGE(TAG, "%s audio peek not writable", __func__);
            }
            break;
        }

        timeBase = (AVRational) {1, avFrame->sample_rate};
        frame->pts = (avFrame->pts == AV_NOPTS_VALUE) ? NAN : avFrame->pts * av_q2d(timeBase);
        frame->pos = avFrame->pkt_pos;
        frame->seekSerial = packetQueue->getFirstSeekSerial();
        frame->duration = av_q2d((AVRational) {avFrame->nb_samples, avFrame->sample_rate});

        av_frame_move_ref(frame->frame, avFrame);

        frameQueue->pushFrame();

        if (DEBUG) {
            ALOGD(TAG, "%s end audio decode frame "
                       "pts = %lf "
                       "pos = %lld "
                       "seekSerial = %d "
                       "duration = %lf "
                       "format = %s "
                       "channel_layout = %lld "
                       "channels = %d ",
                  __func__,
                  frame->pts,
                  frame->pos,
                  frame->seekSerial,
                  frame->duration,
                  av_get_sample_fmt_name((AVSampleFormat) frame->frame->format),
                  frame->frame->channel_layout,
                  frame->frame->channels);
        }
    }

    return SUCCESS;
}

int AudioDecoder::decodeFrameFromPacketQueue(AVFrame *frame) {
    return decodeFrame(frame);
}

int AudioDecoder::decodeFrame(AVFrame *frame) {

    int ret = AVERROR(EAGAIN);

    for (;;) {
        AVPacket packet;

        if (isSamePacketSerial()) {
            // 接收一帧解码后的数据
            do {
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
                if (avcodec_send_packet(codecContext, &packet) == AVERROR(EAGAIN)) {
                    if (DEBUG) {
                        ALOGE(TAG, "%s audio Receive_frame and send_packet both returned EAGAIN, which is an API violation.", __func__);
                    }
                    isPendingPacket = true;
                    av_packet_move_ref(&pendingPacket, &packet);
                }
            }
            av_packet_unref(&packet);
        }
    }
}

bool AudioDecoder::isFinished() {
    return MediaDecoder::isFinished() && getFrameSize() == 0;
}

int64_t AudioDecoder::getFrameQueueLastPos() {
    return frameQueue->currentPos();
}

int AudioDecoder::getAudioFrame(AVFrame *frame) {
    int got_frame = 0;
    int ret = 0;

    if (!frame) {
        return AVERROR(ENOMEM);
    }
    av_frame_unref(frame);

    do {

        if (playerState->abortRequest) {
            return ERROR_ABORT_REQUEST;
        }

        if (playerState->seekRequest) {
            continue;
        }

        AVPacket pkt;
        if (isPendingPacket) {
            av_packet_move_ref(&pkt, &pendingPacket);
            isPendingPacket = false;
        } else {
            if (packetQueue->getPacket(&pkt) < 0) {
                ret = ERROR_ABORT_REQUEST;
                break;
            }
        }

        playerState->mutex.lock();
        // 将数据包解码
        ret = avcodec_send_packet(codecContext, &pkt);
        if (ret < 0) {
            // 一次解码无法消耗完AVPacket中的所有数据，需要重新解码
            if (ret == AVERROR(EAGAIN)) {
                av_packet_move_ref(&pendingPacket, &pkt);
                isPendingPacket = true;
            } else {
                av_packet_unref(&pkt);
                isPendingPacket = false;
            }
            playerState->mutex.unlock();
            continue;
        }

        // 获取解码得到的音频帧AVFrame
        ret = avcodec_receive_frame(codecContext, frame);
        playerState->mutex.unlock();
        // 释放数据包的引用，防止内存泄漏
        av_packet_unref(&pendingPacket);
        if (ret < 0) {
            av_frame_unref(frame);
            got_frame = 0;
            continue;
        } else {
            got_frame = 1;
            // 这里要重新计算frame的pts 否则会导致网络视频出现pts 对不上的情况
            AVRational tb = (AVRational) {1, frame->sample_rate};
            if (frame->pts != AV_NOPTS_VALUE) {
                frame->pts = av_rescale_q(frame->pts, av_codec_get_pkt_timebase(codecContext), tb);
            } else if (nextPts != AV_NOPTS_VALUE) {
                frame->pts = av_rescale_q(nextPts, nextPtsTb, tb);
            }
            if (frame->pts != AV_NOPTS_VALUE) {
                nextPts = frame->pts + frame->nb_samples;
                nextPtsTb = tb;
            }
        }
    } while (!got_frame);

    if (ret < 0) {
        return ERROR;
    }

    return got_frame;
}


