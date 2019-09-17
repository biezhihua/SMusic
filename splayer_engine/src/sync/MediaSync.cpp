#include <sync/MediaSync.h>

MediaSync::MediaSync() {

}

MediaSync::~MediaSync() {

}

void MediaSync::start(VideoDecoder *videoDecoder, AudioDecoder *audioDecoder) {
    this->videoDecoder = videoDecoder;
    this->audioDecoder = audioDecoder;
    videoClock->init(videoDecoder->getPacketQueue()->getPointLastSeekSerial());
    audioClock->init(videoDecoder->getPacketQueue()->getPointLastSeekSerial());
    externalClock->init(videoDecoder->getPacketQueue()->getPointLastSeekSerial());
    abortRequest = false;
}

void MediaSync::stop() {
    ALOGD(TAG, "stop media sync");
    abortRequest = true;
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

void MediaSync::setVideoDevice(VideoDevice *device) {
    Mutex::Autolock lock(mutex);
    this->videoDevice = device;
}

void MediaSync::setMaxDuration(double maxDuration) {
    this->maxFrameDuration = maxDuration;
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
    if (remainingTime > 0.0) {
        av_usleep(static_cast<unsigned int>((int64_t) (remainingTime * 1000000.0)));
    }
    remainingTime = REFRESH_RATE;
    if (playerState != nullptr && videoDecoder != nullptr && videoDevice != nullptr &&
        (!playerState->pauseRequest || forceRefresh)) {
        ALOGD(TAG, "===== refreshVideo =====");
        refreshVideo(&remainingTime);
        ALOGD(TAG, "===== end =====");
    }
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

        if (playerState->abortRequest) {
            ALOGI(TAG, "%s abort request", __func__);
            break;
        }

        // 判断是否存在帧队列是否存在数据
        if (videoDecoder->getFrameSize() > 0) {

            double currentDuration, nextDuration, delay;

            Frame *nextFrame, *currentFrame;

            // 上一帧
            currentFrame = frameQueue->currentFrame();

            // 当前帧
            nextFrame = frameQueue->nextFrame();

            ALOGD(TAG, "nextFrame.seekSerial = %d packetQueue.lastSeekSerial = %d ", nextFrame->seekSerial,
                  packetQueue->getLastSeekSerial());

            // 如果不是相同序列，丢掉seek之前的帧
            if (currentFrame->seekSerial != packetQueue->getLastSeekSerial()) {
                frameQueue->popFrame();
                ALOGE(TAG, "%s drop no same serial of frame", __func__);
                continue;
            }

            ALOGD(TAG, "nextFrame.seekSerial = %d next2Frame.seekSerial = %d",
                  currentFrame->seekSerial,
                  nextFrame->seekSerial);

            // 判断是否需要强制更新帧的时间(seek操作时才会产生变化)
            if (currentFrame->seekSerial != nextFrame->seekSerial) {
                frameTimer = av_gettime_relative() * 1.0F / AV_TIME_BASE;
                ALOGD(TAG, "%s not same serial, force reset frameTimer = %fd ", __func__, frameTimer);
            }

            // 如果处于暂停状态，则直接显示
            if (playerState->abortRequest || playerState->pauseRequest) {
                // to display
                break;
            }

            // 计算帧显示时长
            currentDuration = calculateDuration(currentFrame, nextFrame);

            // 根据帧显示的时长，计算延时
            delay = calculateDelay(currentDuration);

            // 获取当前时间
            time = av_gettime_relative() / 1000000.0;

            // 若上一帧持续显示时间超过当前帧的时间，那么代表上一帧还没显示结束，则继续显示当前帧
            // 如果当前时间小于帧计时器的时间 + 延时时间，则表示还没到当前帧
            if (time < (frameTimer + delay)) {
                *remaining_time = FFMIN(frameTimer + delay - time, *remaining_time);
                ALOGD(TAG, "%s need display pre frame, diff time = %lf remainingTime = %lf", __func__,
                      (frameTimer + delay - time), *remaining_time);
                break;
            }

            // 更新帧计时器
            frameTimer += delay;
            // 帧计时器落后当前时间超过了阈值，则用当前的时间作为帧计时器时间
            if (delay > 0 && (time - frameTimer) > AV_SYNC_THRESHOLD_MAX) {
                frameTimer = time;
                ALOGD(TAG, "%s fall behind, force reset frameTimer = %fd ", __func__, frameTimer);
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
                Frame *next2Frame = frameQueue->next2Frame();
                nextDuration = calculateDuration(nextFrame, next2Frame);

                // 如果不处于同步到视频状态，并且处于跳帧状态，则跳过当前帧
                if ((time > frameTimer + nextDuration)
                    && (playerState->frameDrop > 0
                        || (playerState->frameDrop && playerState->syncType != AV_SYNC_VIDEO))) {
                    frameQueue->popFrame();
                    ALOGD(TAG, "%s drop same frame", __func__);
                    continue;
                }
            }

            // 下一帧
            frameQueue->popFrame();
            forceRefresh = 1;

        } else {
            ALOGW(TAG, "nothing to do, no picture to display in the queue");
        }

        break;
    }

    // 显示画面
    if (!playerState->displayDisable && forceRefresh && videoDecoder && frameQueue->getShowIndex()) {
        ALOGD(TAG, "%s render video", __func__);
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

    if (videoDecoder == nullptr || videoDevice == nullptr) {
        ALOGE(TAG, "%s videoDecoder is null or videoDevice is null", __func__);
        return;
    }

    Frame *currentFrame = videoDecoder->getFrameQueue()->currentFrame();

    // 请求渲染视频
    videoDevice->onRequestRenderStart(currentFrame);

    int ret = 0;

    if (!currentFrame->uploaded) {

        AVFrame *frame = currentFrame->frame;

        TextureFormat format = videoDevice->getTextureFormat(currentFrame->frame->format);
        BlendMode blendMode = videoDevice->getBlendMode(format);

        // 初始化纹理
        if (videoDevice->onInitTexture(0, currentFrame->frame->width, currentFrame->frame->height,
                                       format, blendMode, videoDecoder->getRotate()) < 0) {
            return;
        }

        switch (format) {
            case FMT_YUV420P:
                // 根据图像格式更新纹理数据
                if (frame->linesize[0] > 0 && frame->linesize[1] > 0 && frame->linesize[2] > 0) {
                    ret = videoDevice->onUpdateYUV(frame->data[0], frame->linesize[0],
                                                   frame->data[1], frame->linesize[1],
                                                   frame->data[2], frame->linesize[2]);
                    if (ret < 0) {
                        ALOGE(TAG, "%s update FMT_YUV420P error", __func__);
                        return;
                    }
                } else if (frame->linesize[0] < 0 && frame->linesize[1] < 0 && frame->linesize[2] < 0) {
                    ret = videoDevice->onUpdateYUV(frame->data[0] + frame->linesize[0] * (frame->height - 1),
                                                   -frame->linesize[0],
                                                   frame->data[1] +
                                                   frame->linesize[1] * (AV_CEIL_RSHIFT(frame->height, 1) - 1),
                                                   -frame->linesize[1],
                                                   frame->data[2] +
                                                   frame->linesize[2] * (AV_CEIL_RSHIFT(frame->height, 1) - 1),
                                                   -frame->linesize[2]);
                    if (ret < 0) {
                        ALOGE(TAG, "%s update negative FMT_YUV420P error", __func__);
                        return;
                    }
                }
                break;
            case FMT_ARGB:
                // 直接渲染BGRA，对应的是shader->argb格式
                ret = videoDevice->onUpdateARGB(frame->data[0], frame->linesize[0]);
                if (ret < 0) {
                    ALOGE(TAG, "%s update FMT_ARGB error", __func__);
                    return;
                }
                break;

                // 其他格式转码成BGRA格式再做渲染
            case FMT_NONE:
                swsContext = sws_getCachedContext(swsContext,
                                                  frame->width, frame->height,
                                                  (AVPixelFormat) frame->format,
                                                  frame->width, frame->height,
                                                  AV_PIX_FMT_BGRA, SWS_BICUBIC,
                                                  nullptr, nullptr, nullptr);
                if (!buffer) {
                    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGRA, frame->width, frame->height, 1);
                    buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
                    frameARGB = av_frame_alloc();
                    av_image_fill_arrays(frameARGB->data, frameARGB->linesize, buffer, AV_PIX_FMT_BGRA, frame->width,
                                         frame->height, 1);
                }
                if (swsContext != nullptr) {
                    sws_scale(swsContext, (uint8_t const *const *) frame->data, frame->linesize, 0, frame->height,
                              frameARGB->data, frameARGB->linesize);
                }

                ret = videoDevice->onUpdateARGB(frameARGB->data[0], frameARGB->linesize[0]);

                if (ret < 0) {
                    ALOGE(TAG, "%s update FMT_NONE error", __func__);
                    return;
                }
                break;
            default:
                return;
        }
        currentFrame->uploaded = 1;
    }
    // 请求渲染视频
    videoDevice->onRequestRenderEnd(currentFrame, currentFrame->frame->linesize[0] < 0);
}

void MediaSync::setPlayerState(PlayerState *playerState) {
    MediaSync::playerState = playerState;
}

void MediaSync::resetRemainingTime() {
    remainingTime = 0.0f;
}

void MediaSync::togglePause() {
    if (playerState) {
        if (playerState->pauseRequest) {
            frameTimer += (av_gettime_relative() * 1.0F / AV_TIME_BASE - videoClock->getLastUpdated());
            if (playerState->readPauseReturn != AVERROR(ENOSYS)) {
                videoClock->setPaused(0);
            }
            videoClock->setClock(videoClock->getClock(), videoClock->getSeekSerial());
        }
        externalClock->setClock(externalClock->getClock(), externalClock->getSeekSerial());
        playerState->pauseRequest = !playerState->pauseRequest;
        audioClock->setPaused(playerState->pauseRequest);
        videoClock->setPaused(playerState->pauseRequest);
        externalClock->setPaused(playerState->pauseRequest);
    }
}


int MediaSync::notifyMsg(int what) {
    if (playerState) {
        return playerState->notifyMsg(what);
    }
    return ERROR;
}

int MediaSync::notifyMsg(int what, int arg1) {
    if (playerState) {
        return playerState->notifyMsg(what, arg1);
    }
    return ERROR;
}

int MediaSync::notifyMsg(int what, int arg1, int arg2) {
    if (playerState) {
        return playerState->notifyMsg(what, arg1, arg2);
    }
    return ERROR;
}

void MediaSync::setForceRefresh(int forceRefresh) {
    MediaSync::forceRefresh = forceRefresh;
}

int MediaSync::create() {
    audioClock = new MediaClock();
    videoClock = new MediaClock();
    externalClock = new MediaClock();
    abortRequest = true;
    forceRefresh = 0;
    maxFrameDuration = 10.0;
    frameTimer = 0;
    return SUCCESS;
}

int MediaSync::destroy() {
    delete audioClock;
    audioClock = nullptr;
    delete videoClock;
    videoClock = nullptr;
    delete externalClock;
    externalClock = nullptr;
    return SUCCESS;
}

