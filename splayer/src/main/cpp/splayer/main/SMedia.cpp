
#include <SMedia.h>

SMedia::SMedia(int streamIndex, AVCodec *pCodec, AVCodecParameters *pCodecParameters) {
    this->streamIndex = streamIndex;
    this->pCodec = pCodec;
    this->pCodecParameters = pCodecParameters;
    pthread_mutex_init(&mutex, nullptr);
}

SMedia::~SMedia() {
    streamIndex = -1;
    pCodec = nullptr;
    pCodecParameters = nullptr;
    pthread_mutex_destroy(&mutex);
}

int SMedia::getSampleRate() {
    if (pCodecParameters != nullptr) {
        sampleRate = pCodecParameters->sample_rate;
    }
    return sampleRate;
}

void SMedia::updateTime(AVFrame *pFrame, int dataSize) {
    if (pFrame != nullptr) {
        currentFrameTime = (pFrame->pts * av_q2d(timeBase));
        if (currentFrameTime < currentTime) {
            currentFrameTime = currentTime;
        }
        currentTime = currentFrameTime;
        currentRealTime = currentTime + dataSize / ((double) (getSampleRate() * 2 * 2));
        currentTimeMillis = (currentRealTime * 1000);
    }
    // LOGD("SMedia:updateTime: %f", currentTimeMillis);
}

double SMedia::getCurrentTimeMillis() const {
    return currentTimeMillis;
}

double SMedia::getTotalTimeMillis() const {
    return totalTimeMillis;
}

bool SMedia::isMinDiff() {
    if (currentTimeMillis - lastTimeMillis >= 1000) {
        lastTimeMillis = currentTimeMillis;
        return true;
    }
    return false;
}

double SMedia::getCurrentPTSByAVFrame(AVFrame *avFrame) {
    double pts = av_frame_get_best_effort_timestamp(avFrame);
    if (pts == AV_NOPTS_VALUE) {
        pts = 0;
    }
    pts *= av_q2d(timeBase);
    if (pts > 0) {
        currentFramePTS = pts;
    }
    return currentFramePTS;
}

double SMedia::getCurrentPTSByAVPacket(AVPacket *avPacket) {
    if (avPacket != nullptr) {
        double pts = avPacket->pts;
        if (pts == AV_NOPTS_VALUE) {
            pts = 0;
        }
        pts *= av_q2d(timeBase);
        if (pts > 0) {
            currentFramePTS = pts;
        }
        return currentFramePTS;
    }
    return 0;
}

double SMedia::getCurrentTime() const {
    return currentTime;
}

/**
 * 获取音频与视频的差值
 *
 * 返回值大于0代表视频落后于音频，需要减少睡眠时间
 * 返回值小于0代表视频超前于音频，需要增加睡眠时间
 */
double SMedia::getFrameDiffTime(SMedia *audio, double pts) {
    if (audio != nullptr) {
        double diffTime = audio->getCurrentTime() - pts;
        return diffTime;
    }
    return 0;
}

double SMedia::getDelayRenderTime(double diff) {

    if (diff > 0.003) {
        delayRenderTime = delayRenderTime * 2 / 3;
        if (delayRenderTime < defaultDelayRenderTime / 2) {
            delayRenderTime = defaultDelayRenderTime / 2;
        }
    } else if (diff < -0.003) {
        delayRenderTime = delayRenderTime * 3 / 2;
        if (delayRenderTime > defaultDelayRenderTime * 2) {
            delayRenderTime = defaultDelayRenderTime * 2;
        }
    }

    if (diff >= 0.5) {
        delayRenderTime = 0;
    } else if (diff <= -0.5) {
        delayRenderTime = defaultDelayRenderTime * 2;
    }

    if (fabs(diff) >= 10) {
        delayRenderTime = defaultDelayRenderTime;
    }
    return delayRenderTime;
}

double SMedia::getCurrentRealTime() const {
    return currentRealTime;
}

void SMedia::clearCodecContextBuffer() {
    pthread_mutex_lock(&mutex);
    avcodec_flush_buffers(pCodecContext);
    pthread_mutex_unlock(&mutex);
}

