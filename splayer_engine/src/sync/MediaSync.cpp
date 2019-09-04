
#include <sync/MediaSync.h>

#include "sync/MediaSync.h"

MediaSync::MediaSync() {
    audioDecoder = nullptr;
    videoDecoder = nullptr;
    audioClock = new MediaClock();
    videoClock = new MediaClock();
    externalClock = new MediaClock();

    quit = true;
    abortRequest = true;

    forceRefresh = 0;
    maxFrameDuration = 10.0;
    frameTimerRefresh = 1;
    frameTimer = 0;

    videoDevice = nullptr;
    swsContext = nullptr;
    buffer = nullptr;
    frameARGB = nullptr;
}

MediaSync::~MediaSync() {

}

void MediaSync::reset() {
    stop();
    playerState = nullptr;
    videoDecoder = nullptr;
    audioDecoder = nullptr;
    videoDevice = nullptr;

    if (frameARGB) {
        av_frame_free(&frameARGB);
        av_free(frameARGB);
        frameARGB = nullptr;
    }
    if (buffer) {
        av_freep(&buffer);
        buffer = nullptr;
    }
    if (swsContext) {
        sws_freeContext(swsContext);
        swsContext = nullptr;
    }
}

void MediaSync::start(VideoDecoder *videoDecoder, AudioDecoder *audioDecoder) {
    mutex.lock();
    this->videoDecoder = videoDecoder;
    this->audioDecoder = audioDecoder;
    videoClock->init(videoDecoder->getPacketQueue()->getPointLastSeekSerial());
    audioClock->init(videoDecoder->getPacketQueue()->getPointLastSeekSerial());
    externalClock->init(videoDecoder->getPacketQueue()->getPointLastSeekSerial());
    abortRequest = false;
    quit = false;
    condition.signal();
    mutex.unlock();
}

void MediaSync::stop() {
    mutex.lock();
    abortRequest = true;
    condition.signal();
    mutex.unlock();

    mutex.lock();
    while (!quit) {
        condition.wait(mutex);
    }
    mutex.unlock();
}

void MediaSync::setVideoDevice(VideoDevice *device) {
    Mutex::Autolock lock(mutex);
    this->videoDevice = device;
}

void MediaSync::setMaxDuration(double maxDuration) {
    this->maxFrameDuration = maxDuration;
}

void MediaSync::refreshVideoTimer() {
    mutex.lock();
    this->frameTimerRefresh = 1;
    condition.signal();
    mutex.unlock();
}

void MediaSync::updateAudioClock(double pts, double time) {
    audioClock->setClock(pts, time);
    externalClock->syncToSlave(audioClock);
}

double MediaSync::getAudioDiffClock() {
    return audioClock->getClock() - getMasterClock();
}

void MediaSync::updateExternalClock(double pts, int seekSerial) {
    externalClock->setClock(pts, seekSerial);
}

double MediaSync::getMasterClock() {
    double val = 0;
    switch (playerState->syncType) {
        case AV_SYNC_VIDEO: {
            val = videoClock->getClock();
            break;
        }
        case AV_SYNC_AUDIO: {
            val = audioClock->getClock();
            break;
        }
        case AV_SYNC_EXTERNAL: {
            val = externalClock->getClock();
            break;
        }
    }
    return val;
}

MediaClock *MediaSync::getAudioClock() {
    return audioClock;
}

MediaClock *MediaSync::getVideoClock() {
    return videoClock;
}

MediaClock *MediaSync::getExternalClock() {
    return externalClock;
}

void MediaSync::run() {

}

void MediaSync::refreshVideo() {
    if (abortRequest || playerState->abortRequest) {
        if (videoDevice != nullptr) {
            videoDevice->terminate();
        }
        return;
    }
    ALOGD(TAG, "===== refreshVideo =====");
    ALOGD(TAG, "%s while remainingTime = %lf pauseRequest = %d forceRefresh = %d", __func__, remainingTime,
          playerState->pauseRequest, forceRefresh);
    if (remainingTime > 0.0) {
        av_usleep(static_cast<unsigned int>((int64_t) (remainingTime * 1000000.0)));
    }
    remainingTime = REFRESH_RATE;
    if (!playerState->pauseRequest || forceRefresh) {
        refreshVideo(&remainingTime);
    }
    ALOGD(TAG, "===== end =====");
}

void MediaSync::refreshVideo(double *remaining_time) {
    double time;

    // 检查外部时钟
    if (!playerState->pauseRequest && playerState->realTime && playerState->syncType == AV_SYNC_EXTERNAL) {
        checkExternalClockSpeed();
    }

    FrameQueue *frameQueue = videoDecoder->getFrameQueue();
    PacketQueue *packetQueue = videoDecoder->getPacketQueue();

    for (;;) {

        if (playerState->abortRequest || !videoDecoder) {
            break;
        }

        // 判断是否存在帧队列是否存在数据
        if (videoDecoder->getFrameSize() > 0) {

            double currentDuration, nextDuration, delay;

            Frame *nextFrame, *currentFrame;

            // 上一帧
            currentFrame = frameQueue->lastFrame();

            // 当前帧
            nextFrame = frameQueue->currentFrame();

            ALOGD(TAG, "currentFrame seekSerial = %d packetQueue lastSeekSerial = %d ", nextFrame->seekSerial, packetQueue->getLastSeekSerial());

            if (currentFrame->seekSerial != packetQueue->getLastSeekSerial()) {
                frameQueue->popFrame();
                ALOGE(TAG, "goto retry, need to sync seekSerial");
                break;
            }

            ALOGD(TAG, "currentFrame seekSerial = %d nextFrame seekSerial = %d ", currentFrame->seekSerial,
                  nextFrame->seekSerial);

            // seek操作时才会产生变化
            // 判断是否需要强制更新帧的时间
            if (frameTimerRefresh || currentFrame->seekSerial != nextFrame->seekSerial) {
                frameTimer = av_gettime_relative() * 1.0F / AV_TIME_BASE;
                ALOGD(TAG, "update frameTimer = %fd ", frameTimer);
                frameTimerRefresh = 0;
            }

            // 如果处于暂停状态，则直接显示
            if (playerState->abortRequest || playerState->pauseRequest) {
                break;
            }

            // 计算上一次显示时长
            currentDuration = calculateDuration(currentFrame, nextFrame);

            // 根据上一次显示的时长，计算延时
            delay = calculateDelay(currentDuration);


            // 获取当前时间
            time = av_gettime_relative() / 1000000.0;

            // 如果当前时间小于帧计时器的时间 + 延时时间，则表示还没到当前帧
            if (time < frameTimer + delay) {
                *remaining_time = FFMIN(frameTimer + delay - time, *remaining_time);
                continue;
            }

            // 更新帧计时器
            frameTimer += delay;
            // 帧计时器落后当前时间超过了阈值，则用当前的时间作为帧计时器时间
            if (delay > 0 && time - frameTimer > AV_SYNC_THRESHOLD_MAX) {
                frameTimer = time;
            }

            // 更新视频时钟的pts
            mutex.lock();
            if (!isnan(nextFrame->pts)) {
                videoClock->setClock(nextFrame->pts, nextFrame->seekSerial);
                externalClock->syncToSlave(videoClock);
            }
            mutex.unlock();

            // 如果队列中还剩余超过一帧的数据时，需要拿到下一帧，然后计算间隔，并判断是否需要进行舍帧操作
            if (videoDecoder->getFrameSize() > 1) {
                Frame *nextNextFrame = frameQueue->nextFrame();
                nextDuration = calculateDuration(nextFrame, nextNextFrame);

                // 如果不处于同步到视频状态，并且处于跳帧状态，则跳过当前帧
                if ((time > frameTimer + nextDuration)
                    && (playerState->frameDrop > 0
                        || (playerState->frameDrop && playerState->syncType != AV_SYNC_VIDEO))) {
                    frameQueue->popFrame();
                    ALOGD(TAG, "%s drop frame late", __func__);
                    continue;
                }
            }

            // 下一帧
            frameQueue->popFrame();
            forceRefresh = 1;

        } else {
            ALOGD(TAG, "nothing to do, no picture to display in the queue");
        }

        break;
    }

    // 显示画面
    if (!playerState->displayDisable && forceRefresh && videoDecoder && frameQueue->getShowIndex()) {
        renderVideo();
    }
    forceRefresh = 0;
}

void MediaSync::checkExternalClockSpeed() {
    if ((videoDecoder && videoDecoder->getPacketSize() <= EXTERNAL_CLOCK_MIN_FRAMES)
        || (audioDecoder && audioDecoder->getPacketSize() <= EXTERNAL_CLOCK_MIN_FRAMES)) {
        externalClock->setSpeed(FFMAX(EXTERNAL_CLOCK_SPEED_MIN,
                                      externalClock->getSpeed() - EXTERNAL_CLOCK_SPEED_STEP));
    } else if ((!videoDecoder || videoDecoder->getPacketSize() > EXTERNAL_CLOCK_MAX_FRAMES)
               && (!audioDecoder || audioDecoder->getPacketSize() > EXTERNAL_CLOCK_MAX_FRAMES)) {
        externalClock->setSpeed(FFMIN(EXTERNAL_CLOCK_SPEED_MAX,
                                      externalClock->getSpeed() + EXTERNAL_CLOCK_SPEED_STEP));
    } else {
        double speed = externalClock->getSpeed();
        if (speed != 1.0) {
            externalClock->setSpeed(speed + EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / fabs(1.0 - speed));
        }
    }
}

double MediaSync::calculateDelay(double delay) {
    double sync_threshold, diff = 0;

    // 如果不是同步到视频流，则需要计算延时时间
    // 如果不是以视频做为同步基准，则计算延时
    if (playerState->syncType != AV_SYNC_VIDEO) {

        // 计算差值
        diff = videoClock->getClock() - getMasterClock();

        // 用差值与同步阈值计算延时
        // skip or repeat frame. We take into account the duration to compute the threshold. I still don't know
        // if it is the best guess */
        // 0.04 ~ 0.1
        // 计算同步阈值
        sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));

        ALOGD(TAG, "%s diff = %lf syncThreshold[0.04,0.1] = %lf ", __func__, diff, sync_threshold);

        // 判断时间差是否在许可范围内
        if (!isnan(diff) && fabs(diff) < maxFrameDuration) {
            if (diff <= -sync_threshold) {
                // 滞后
                delay = FFMAX(0, delay + diff);
            } else if (diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD) {
                // 超前
                delay = delay + diff;
            } else if (diff >= sync_threshold) {
                // 超出了理论阈值
                delay = 2 * delay;
            }
        }
    }

    ALOGD(TAG, "%s video: delay=%0.3f A-V=%f", __func__, delay, -diff);

    return delay;
}

double MediaSync::calculateDuration(Frame *current, Frame *next) {
    if (current->seekSerial == next->seekSerial) {
        double duration = next->pts - current->pts;
        if (isnan(duration) || duration <= 0 || duration > maxFrameDuration) {
            return current->duration;
        } else {
            return duration;
        }
    }
    return 0.0;
}

void MediaSync::renderVideo() {
    mutex.lock();
    if (!videoDecoder || !videoDevice) {
        mutex.unlock();
        return;
    }
    Frame *vp = videoDecoder->getFrameQueue()->lastFrame();
    int ret = 0;
    if (!vp->uploaded) {
        // 根据图像格式更新纹理数据
        switch (vp->frame->format) {
            // YUV420P 和 YUVJ420P 除了色彩空间不一样之外，其他的没什么区别
            // YUV420P表示的范围是 16 ~ 235，而YUVJ420P表示的范围是0 ~ 255
            // 这里做了兼容处理，后续可以优化，shader已经过验证
            case AV_PIX_FMT_YUVJ420P:
            case AV_PIX_FMT_YUV420P: {

                // 初始化纹理
                videoDevice->onInitTexture(vp->frame->width, vp->frame->height,
                                           FMT_YUV420P, BLEND_NONE);

                if (vp->frame->linesize[0] < 0 || vp->frame->linesize[1] < 0 || vp->frame->linesize[2] < 0) {
                    av_log(nullptr, AV_LOG_ERROR, "Negative linesize is not supported for YUV.\n");
                    return;
                }
                ret = videoDevice->onUpdateYUV(vp->frame->data[0], vp->frame->linesize[0],
                                               vp->frame->data[1], vp->frame->linesize[1],
                                               vp->frame->data[2], vp->frame->linesize[2]);
                if (ret < 0) {
                    return;
                }
                break;
            }

                // 直接渲染BGRA，对应的是shader->argb格式
            case AV_PIX_FMT_BGRA: {
                videoDevice->onInitTexture(vp->frame->width, vp->frame->height,
                                           FMT_ARGB, BLEND_NONE);
                ret = videoDevice->onUpdateARGB(vp->frame->data[0], vp->frame->linesize[0]);
                if (ret < 0) {
                    return;
                }
                break;
            }

                // 其他格式转码成BGRA格式再做渲染
            default: {
                swsContext = sws_getCachedContext(swsContext,
                                                  vp->frame->width, vp->frame->height,
                                                  (AVPixelFormat) vp->frame->format,
                                                  vp->frame->width, vp->frame->height,
                                                  AV_PIX_FMT_BGRA, SWS_BICUBIC, nullptr, nullptr, nullptr);
                if (!buffer) {
                    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGRA, vp->frame->width, vp->frame->height, 1);
                    buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
                    frameARGB = av_frame_alloc();
                    av_image_fill_arrays(frameARGB->data, frameARGB->linesize, buffer, AV_PIX_FMT_BGRA,
                                         vp->frame->width, vp->frame->height, 1);
                }
                if (swsContext != nullptr) {
                    sws_scale(swsContext, (uint8_t const *const *) vp->frame->data,
                              vp->frame->linesize, 0, vp->frame->height,
                              frameARGB->data, frameARGB->linesize);
                }

                videoDevice->onInitTexture(vp->frame->width, vp->frame->height,
                                           FMT_ARGB, BLEND_NONE, videoDecoder->getRotate());
                ret = videoDevice->onUpdateARGB(frameARGB->data[0], frameARGB->linesize[0]);
                if (ret < 0) {
                    return;
                }
                break;
            }
        }
        vp->uploaded = 1;
    }
    // 请求渲染视频
    if (videoDevice != nullptr) {
        videoDevice->onRequestRender(vp->frame->linesize[0] < 0);
    }
    mutex.unlock();
}

void MediaSync::setPlayerState(PlayerState *playerState) {
    MediaSync::playerState = playerState;
}

void MediaSync::resetRemainingTime() {
    remainingTime = 0.0f;
}
