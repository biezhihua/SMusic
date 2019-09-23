#include "decoder/AudioDecoder.h"

AudioDecoder::AudioDecoder(AVCodecContext *avctx,
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
}

AudioDecoder::~AudioDecoder() {
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
                ret = -1;
                break;
            }
        }

        playerState->mMutex.lock();
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
            playerState->mMutex.unlock();
            continue;
        }

        // 获取解码得到的音频帧AVFrame
        ret = avcodec_receive_frame(codecContext, frame);
        playerState->mMutex.unlock();
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
        return -1;
    }

    return got_frame;
}






