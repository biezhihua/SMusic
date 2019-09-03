#include "decoder/VideoDecoder.h"

VideoDecoder::VideoDecoder(AVFormatContext *pFormatCtx, AVCodecContext *avctx,
                           AVStream *stream, int streamIndex, PlayerState *playerState, AVPacket *flushPacket)
        : MediaDecoder(avctx, stream, streamIndex, playerState, flushPacket) {
    formatContext = pFormatCtx;
    frameQueue = new FrameQueue(VIDEO_QUEUE_SIZE, 1);
    quit = true;
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
    mutex.lock();
    formatContext = nullptr;
    if (frameQueue) {
        frameQueue->flush();
        delete frameQueue;
        frameQueue = nullptr;
    }
    masterClock = nullptr;
    mutex.unlock();
}

void VideoDecoder::setMasterClock(MediaClock *masterClock) {
    Mutex::Autolock lock(mutex);
    this->masterClock = masterClock;
}

void VideoDecoder::start() {
    MediaDecoder::start();
    if (frameQueue) {
        frameQueue->start();
    }
    if (!decodeThread) {
        decodeThread = new Thread(this);
        decodeThread->start();
        quit = false;
    }
}

void VideoDecoder::stop() {
    MediaDecoder::stop();
    if (frameQueue) {
        frameQueue->abort();
    }
    mutex.lock();
    while (!quit) {
        condition.wait(mutex);
    }
    mutex.unlock();
    if (decodeThread) {
        decodeThread->join();
        delete decodeThread;
        decodeThread = nullptr;
    }
}

void VideoDecoder::flush() {
    mutex.lock();
    MediaDecoder::flush();
    if (frameQueue) {
        frameQueue->flush();
    }
    condition.signal();
    mutex.unlock();
}

int VideoDecoder::getFrameSize() {
    Mutex::Autolock lock(mutex);
    return frameQueue ? frameQueue->getFrameSize() : 0;
}

int VideoDecoder::getRotate() {
    Mutex::Autolock lock(mutex);
    return rotate;
}

FrameQueue *VideoDecoder::getFrameQueue() {
    Mutex::Autolock lock(mutex);
    return frameQueue;
}

AVFormatContext *VideoDecoder::getFormatContext() {
    Mutex::Autolock lock(mutex);
    return formatContext;
}

void VideoDecoder::run() {
    decodeVideo();
}

/**
 * 解码视频数据包并放入帧队列
 * @return
 */
int VideoDecoder::decodeVideo() {
    AVFrame *frame = av_frame_alloc();
    Frame *vp;
    int got_picture;
    int ret = 0;

    AVRational tb = stream->time_base;
    AVRational frame_rate = av_guess_frame_rate(formatContext, stream, nullptr);

    if (!frame) {
        quit = true;
        condition.signal();
        return AVERROR(ENOMEM);
    }

    AVPacket *packet = av_packet_alloc();
    if (!packet) {
        quit = true;
        condition.signal();
        return AVERROR(ENOMEM);
    }

    for (;;) {

        if (abortRequest || playerState->abortRequest) {
            ret = -1;
            break;
        }

        if (playerState->seekRequest) {
            continue;
        }

        if (packetQueue->getPacket(packet) < 0) {
            ret = -1;
            break;
        }

        // 送去解码
        playerState->mMutex.lock();
        ret = avcodec_send_packet(codecContext, packet);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            av_packet_unref(packet);
            playerState->mMutex.unlock();
            continue;
        }

        // 得到解码帧
        ret = avcodec_receive_frame(codecContext, frame);
        playerState->mMutex.unlock();
        if (ret < 0 && ret != AVERROR_EOF) {
            av_frame_unref(frame);
            av_packet_unref(packet);
            continue;
        } else {
            got_picture = 1;

            // 是否重排pts，默认情况下需要重排pts的
            if (playerState->reorderVideoPts == -1) {
                frame->pts = av_frame_get_best_effort_timestamp(frame);
            } else if (!playerState->reorderVideoPts) {
                frame->pts = frame->pkt_dts;
            }

            // 丢帧处理
            if (masterClock != nullptr) {
                double dpts = NAN;

                if (frame->pts != AV_NOPTS_VALUE) {
                    dpts = av_q2d(stream->time_base) * frame->pts;
                }
                // 计算视频帧的长宽比
                frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(formatContext, stream,
                                                                          frame);
                // 是否需要做舍帧操作
                if (playerState->frameDrop > 0 ||
                    (playerState->frameDrop > 0 && playerState->syncType != AV_SYNC_VIDEO)) {
                    if (frame->pts != AV_NOPTS_VALUE) {
                        double diff = dpts - masterClock->getClock();
                        if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD &&
                            diff < 0 && packetQueue->getPacketSize() > 0) {
                            av_frame_unref(frame);
                            got_picture = 0;
                        }
                    }
                }
            }
        }

        if (got_picture) {

            // 取出帧
            if (!(vp = frameQueue->peekWritable())) {
                ret = -1;
                break;
            }

            // 复制参数
            vp->uploaded = 0;
            vp->width = frame->width;
            vp->height = frame->height;
            vp->format = frame->format;
            vp->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
            vp->duration = frame_rate.num && frame_rate.den
                           ? av_q2d((AVRational) {frame_rate.den, frame_rate.num}) : 0;
            av_frame_move_ref(vp->frame, frame);

            // 入队帧
            frameQueue->pushFrame();
        }

        // 释放数据包和缓冲帧的引用，防止内存泄漏
        av_frame_unref(frame);
        av_packet_unref(packet);
    }

    av_frame_free(&frame);
    av_free(frame);
    frame = nullptr;

    av_packet_free(&packet);
    av_free(packet);
    packet = nullptr;

    quit = true;
    condition.signal();

    return ret;
}

bool VideoDecoder::isFinished() {
    return MediaDecoder::isFinished() && getFrameSize() == 0;
}

