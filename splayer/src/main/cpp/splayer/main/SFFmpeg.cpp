#include <SFFmpeg.h>

#define FFMPEG_TAG "Native_FFmpeg"

SFFmpeg::SFFmpeg(SStatus *pStatus, SJavaMethods *pJavaMethods) {
    this->pStatus = pStatus;
    this->pJavaMethods = pJavaMethods;
    this->pAudioQueue = new SQueue();
    this->pVideoQueue = new SQueue();

    pthread_mutex_init(&seekMutex, nullptr);
}

SFFmpeg::~SFFmpeg() {

    pthread_mutex_destroy(&seekMutex);

    delete pSource;
    pSource = nullptr;

    if (pVideoQueue != nullptr) {
        pVideoQueue->clear();
        delete pVideoQueue;
        pVideoQueue = nullptr;
    }

    if (pAudioQueue != nullptr) {
        pAudioQueue->clear();
        delete pAudioQueue;
        pAudioQueue = nullptr;
    }

    release();

    delete pVideoMedia;
    pVideoMedia = nullptr;

    delete pAudioMedia;
    pAudioMedia = nullptr;

    pStatus = nullptr;

    pJavaMethods = nullptr;
}

void SFFmpeg::setSource(string *source) {
//    LOGD(FFMPEG_TAG, "setSource: %s ", source);
    if (pSource != nullptr) {
        delete pSource;
        pSource = nullptr;
    }
    if (source != nullptr) {
        pSource = new string(source->c_str());
    }
}

int SFFmpeg::decodeMediaInfo() {

    pBuffer = (uint8_t *) (av_malloc(44100 * 2 * 2));

    // AVFormatContext holds the header information from the format (Container)
    // Allocating memory for this component
    // http://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
    pFormatContext = avformat_alloc_context();

    if (pFormatContext == nullptr) {
        LOGE(FFMPEG_TAG, "decodeMediaInfo: ERROR could not allocate memory for Format Context");
        return S_ERROR;
    }

    if (pSource == nullptr) {
        LOGE(FFMPEG_TAG, "decodeMediaInfo: ERROR source is empty");
        return S_ERROR;
    }

    LOGD(FFMPEG_TAG, "decodeMediaInfo: Opening the input file (%s) and _isLoading format (container) header",
         pSource->c_str());

    // Open the file and read its header. The codecs are not opened.
    // The function arguments are:
    // AVFormatContext (the component we allocated memory for),
    // url (filename),
    // AVInputFormat (if you pass nullptr it'll do the auto detect)
    // and AVDictionary (which are options to the demuxer)
    // http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga31d601155e9035d5b0e7efedc894ee49
    int result = avformat_open_input(&pFormatContext, pSource->c_str(), nullptr, nullptr);
    if (result != 0) {
        LOGE(FFMPEG_TAG, "decodeMediaInfo: ERROR could not open the file %d", result);
        return S_ERROR;
    }

    // now we have access to some information about our file
    // since we read its header we can say what format (container) it's
    // and some other information related to the format itself.
    LOGD(FFMPEG_TAG, "decodeMediaInfo: Format %s, totalTime %lld us, bit_rate %lld", pFormatContext->iformat->name,
         pFormatContext->duration,
         pFormatContext->bit_rate);

    LOGD(FFMPEG_TAG, "decodeMediaInfo: Finding stream info from format");
    // read Packets from the Format to get stream information
    // this function populates pFormatContext->streams
    // (of size equals to pFormatContext->nb_streams)
    // the arguments are:
    // the AVFormatContext
    // and options contains options for codec corresponding to i-th stream.
    // On return each dictionary will be filled with options that were not found.
    // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#gad42172e27cddafb81096939783b157bb
    if (avformat_find_stream_info(pFormatContext, nullptr) < 0) {
        LOGD(FFMPEG_TAG, "decodeMediaInfo: ERROR could not get the stream info");
        return S_ERROR;
    }

    // loop though all the streams and print its main information
    for (int i = 0; i < pFormatContext->nb_streams; i++) {

        LOGD(FFMPEG_TAG, "decodeMediaInfo: ----------------------- ");

        AVCodecParameters *pLocalCodecParameters = nullptr;
        pLocalCodecParameters = pFormatContext->streams[i]->codecpar;

        LOGD(FFMPEG_TAG, "decodeMediaInfo: AVStream->time_base before open coded %d/%d",
             pFormatContext->streams[i]->time_base.num,
             pFormatContext->streams[i]->time_base.den);

        LOGD(FFMPEG_TAG, "decodeMediaInfo: AVStream->r_frame_rate before open coded %d/%d",
             pFormatContext->streams[i]->r_frame_rate.num,
             pFormatContext->streams[i]->r_frame_rate.den);

//        LOGD(FFMPEG_TAG, "decodeMediaInfo: AVStream->start_time %d", pFormatContext->streams[i]->start_time);
//        LOGD(FFMPEG_TAG, "decodeMediaInfo: AVStream->totalTime %d", pFormatContext->streams[i]->duration);

        LOGD(FFMPEG_TAG, "decodeMediaInfo: Finding the proper decoder (CODEC)");

        // when the stream is a video we store its index, codec parameters and codec
        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {

            AVCodec *pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

            if (pLocalCodec == nullptr) {
                LOGD(FFMPEG_TAG, "decodeMediaInfo: ERROR unsupported codec!");
                continue;
            }

            pVideoMedia = new SMedia(i, pLocalCodec, pLocalCodecParameters);
            pVideoMedia->totalTime = (pFormatContext->duration / AV_TIME_BASE);
            pVideoMedia->totalTimeMillis = pVideoMedia->totalTime * 1000;
            pVideoMedia->timeBase = (pFormatContext->streams[pVideoMedia->streamIndex]->time_base);
            int num = pFormatContext->streams[pVideoMedia->streamIndex]->avg_frame_rate.num;
            int den = pFormatContext->streams[pVideoMedia->streamIndex]->avg_frame_rate.den;
            if (num != 0 && den != 0) {
                int fps = num / den;
                pVideoMedia->defaultDelayRenderTime = 1.0f / fps;
                LOGD(FFMPEG_TAG, "decodeMediaInfo: defaultDelayRenderTime %lf", pVideoMedia->defaultDelayRenderTime);
            }

            LOGD(FFMPEG_TAG, "decodeMediaInfo: Video Codec: resolution %d x %d", pLocalCodecParameters->width,
                 pLocalCodecParameters->height);
            // print its name, id and bitrate
            LOGD(FFMPEG_TAG, "decodeMediaInfo: Codec %s ID %d bit_rate %lld", pLocalCodec->name, pLocalCodec->id,
                 pLocalCodecParameters->bit_rate);

        } else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {

            AVCodec *pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

            if (pLocalCodec == nullptr) {
                LOGD(FFMPEG_TAG, "decodeMediaInfo: ERROR unsupported codec!");
                continue;
            }

            pAudioMedia = new SMedia(i, pLocalCodec, pLocalCodecParameters);
            pAudioMedia->totalTime = ((long) (pFormatContext->duration / AV_TIME_BASE));
            pAudioMedia->totalTimeMillis = pAudioMedia->totalTime * 1000;
            pAudioMedia->timeBase = (pFormatContext->streams[pAudioMedia->streamIndex]->time_base);

            LOGD(FFMPEG_TAG, "decodeMediaInfo: Audio Codec: %d channels, sample rate %d",
                 pLocalCodecParameters->channels,
                 pLocalCodecParameters->sample_rate);
            LOGD(FFMPEG_TAG, "decodeMediaInfo: Codec %s ID %d bit_rate %lld", pLocalCodec->name, pLocalCodec->id,
                 pLocalCodecParameters->bit_rate);
        }
    }

    LOGD(FFMPEG_TAG, "decodeMediaInfo: ----------------------- ");

    if (pAudioMedia != nullptr) {
        // https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
        pAudioMedia->pCodecContext = (avcodec_alloc_context3(pAudioMedia->pCodec));
        if (pAudioMedia->pCodecContext == nullptr) {
            LOGE(FFMPEG_TAG, "decodeMediaInfo: Failed to allocated memory for AVCodecContext");
            return S_ERROR;
        }

        // Fill the codec context based on the values from the supplied codec parameters
        // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
        if (avcodec_parameters_to_context(pAudioMedia->pCodecContext, pAudioMedia->pCodecParameters) < 0) {
            LOGE(FFMPEG_TAG, "decodeMediaInfo: Failed to copy codec params to codec context");
            return S_ERROR;
        }

        // Initialize the AVCodecContext to use the given AVCodec.
        // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
        if (avcodec_open2(pAudioMedia->pCodecContext, pAudioMedia->pCodec, nullptr) < 0) {
            LOGE(FFMPEG_TAG, "decodeMediaInfo: Failed to open codec through avcodec_open2");
            return S_ERROR;
        }
    }

    LOGD(FFMPEG_TAG, "decodeMediaInfo: Video ----------------------- ");
    if (pVideoMedia != nullptr) {
        // https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
        pVideoMedia->pCodecContext = (avcodec_alloc_context3(pVideoMedia->pCodec));
        if (pVideoMedia->pCodecContext == nullptr) {
            LOGE(FFMPEG_TAG, "decodeMediaInfo: Failed to allocated memory for AVCodecContext");
            return S_ERROR;
        }

        // Fill the codec context based on the values from the supplied codec parameters
        // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
        if (avcodec_parameters_to_context(pVideoMedia->pCodecContext, pVideoMedia->pCodecParameters) < 0) {
            LOGE(FFMPEG_TAG, "decodeMediaInfo: Failed to copy codec params to codec context");
            return S_ERROR;
        }

        // Initialize the AVCodecContext to use the given AVCodec.
        // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
        if (avcodec_open2(pVideoMedia->pCodecContext, pVideoMedia->pCodec, nullptr) < 0) {
            LOGE(FFMPEG_TAG, "decodeMediaInfo: Failed to open codec through avcodec_open2");
            return S_ERROR;
        }
    }

    return S_SUCCESS;
}


int SFFmpeg::decodeFrame() {

    if (pStatus != nullptr && pStatus->isSeek()) {
        sleep();
        return S_FUNCTION_CONTINUE;
    }

    if (pVideoMedia == nullptr && pAudioMedia == nullptr) {
        return S_FUNCTION_BREAK;
    }

    bool isOnlyAudio = pAudioMedia != nullptr &&
                       pVideoMedia == nullptr &&
                       pAudioQueue != nullptr &&
                       pAudioQueue->getSize() > CACHE_SIZE;
    bool isOnlyVideo = pAudioMedia == nullptr &&
                       pVideoMedia != nullptr &&
                       pVideoQueue != nullptr &&
                       pVideoQueue->getSize() > CACHE_SIZE;

    bool isAudioAndVideo = pAudioMedia != nullptr &&
                           pAudioQueue != nullptr &&
                           pVideoMedia != nullptr &&
                           pVideoQueue != nullptr &&
                           pAudioQueue->getSize() > CACHE_SIZE &&
                           pVideoQueue->getSize() > CACHE_SIZE;

    if (isOnlyAudio || isOnlyVideo || isAudioAndVideo) {
        sleep();
        LOGE(FFMPEG_TAG, "decodeFrame: Size OverFlow Sleep");
        return S_FUNCTION_CONTINUE;
    }

    // https://ffmpeg.org/doxygen/trunk/structAVPacket.html
    pDecodeRawFramePacket = av_packet_alloc();
    if (pDecodeRawFramePacket == nullptr) {
        LOGE(FFMPEG_TAG, "decodeFrame: failed to allocated memory for AVPacket");
        return S_FUNCTION_BREAK;
    }

    pthread_mutex_lock(&seekMutex);
    int ret = av_read_frame(pFormatContext, pDecodeRawFramePacket);
    pthread_mutex_unlock(&seekMutex);

    if (ret == 0) {
        if (pAudioMedia != nullptr && pDecodeRawFramePacket->stream_index == pAudioMedia->streamIndex) {
            pAudioQueue->putAvPacket(pDecodeRawFramePacket);
            LOGD(FFMPEG_TAG, "decodeFrame: pAudioQueue putAvPacket %d", pAudioQueue->getSize());
        } else if (pVideoMedia != nullptr && pDecodeRawFramePacket->stream_index == pVideoMedia->streamIndex) {
            pVideoQueue->putAvPacket(pDecodeRawFramePacket);
            LOGD(FFMPEG_TAG, "decodeFrame: pVideoQueue putAvPacket %d", pVideoQueue->getSize());
        }

        return S_SUCCESS;
    } else {
        av_packet_free(&pDecodeRawFramePacket);
        av_free(pDecodeRawFramePacket);
        if (pStatus->isPlay()) {
            LOGE(FFMPEG_TAG, "decodeFrame: decode finished continue");
            return S_FUNCTION_CONTINUE;
        }
        LOGE(FFMPEG_TAG, "decodeFrame: decode finished read frame ret=%d", ret);
        return S_FUNCTION_BREAK;
    }
}


int SFFmpeg::softDecodeVideo() {
    if (pVideoMedia == nullptr || pVideoQueue == nullptr) {
        return S_FUNCTION_BREAK;
    }

    pVideoPacket = av_packet_alloc();

    if (pVideoPacket == nullptr) {
        LOGE(FFMPEG_TAG, "softDecodeVideo: failed to allocated memory for AVPacket");
        return S_FUNCTION_BREAK;
    }

    if (getVideoAvPacketFromAudioQueue(pVideoPacket) == S_ERROR) {
        releasePacket(&pVideoPacket);
        releaseFrame(&pVideoFrame);
        return S_FUNCTION_CONTINUE;
    }

    pthread_mutex_lock(&pVideoMedia->mutex);

    int result = avcodec_send_packet(pVideoMedia->pCodecContext, pVideoPacket);

    if (result != S_SUCCESS) {
        pthread_mutex_unlock(&pVideoMedia->mutex);
        LOGE(FFMPEG_TAG, "softDecodeVideo: send packet failed");
        return S_FUNCTION_CONTINUE;
    }

    pVideoFrame = av_frame_alloc();
    if (pVideoFrame == nullptr) {
        pthread_mutex_unlock(&pVideoMedia->mutex);
        LOGE(FFMPEG_TAG, "softDecodeVideo: ailed to allocated memory for AVFrame");
        return S_FUNCTION_BREAK;
    }

    result = avcodec_receive_frame(pVideoMedia->pCodecContext, pVideoFrame);

    if (result != S_SUCCESS) {
        releasePacket(&pVideoPacket);
        releaseFrame(&pVideoFrame);
        pthread_mutex_unlock(&pVideoMedia->mutex);
        LOGE(FFMPEG_TAG, "softDecodeVideo: receive frame failed %d", result);
        return S_FUNCTION_CONTINUE;
    }

    if (pVideoFrame->format == AV_PIX_FMT_YUV420P) {

        double diffTime = pVideoMedia->getFrameDiffTime(pAudioMedia, pVideoMedia->getCurrentPTSByAVFrame(pVideoFrame));

        double delayTime = pVideoMedia->getDelayRenderTime(diffTime);

        LOGD(FFMPEG_TAG, "softDecodeVideo diffTime %lf delayTime %lf", diffTime, delayTime);

        av_usleep(static_cast<unsigned int>(delayTime * 1000000));

        if (pJavaMethods != nullptr && pVideoMedia != nullptr && pVideoMedia->pCodecContext != nullptr) {
            pJavaMethods->onCallJavaRenderYUVFromThread(
                    pVideoMedia->pCodecContext->width,
                    pVideoMedia->pCodecContext->height,
                    pVideoFrame->data[0],
                    pVideoFrame->data[1],
                    pVideoFrame->data[2]
            );
        }
    } else {
        AVFrame *pFrameYUV420P = av_frame_alloc();
        int num = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                           pVideoMedia->pCodecContext->width,
                                           pVideoMedia->pCodecContext->height, 1);
        uint8_t *buffer = static_cast<uint8_t *>(av_malloc(num * sizeof(uint8_t)));
        av_image_fill_arrays(pFrameYUV420P->data, pFrameYUV420P->linesize,
                             buffer,
                             AV_PIX_FMT_YUV420P,
                             pVideoMedia->pCodecContext->width,
                             pVideoMedia->pCodecContext->height, 1);

        SwsContext *swsContext = sws_getContext(
                pVideoMedia->pCodecContext->width,
                pVideoMedia->pCodecContext->height,
                pVideoMedia->pCodecContext->pix_fmt,
                pVideoMedia->pCodecContext->width,
                pVideoMedia->pCodecContext->height,
                AV_PIX_FMT_YUV420P,
                SWS_BICUBIC, nullptr, nullptr, nullptr
        );

        if (swsContext == nullptr) {
            av_frame_free(&pFrameYUV420P);
            av_free(&pFrameYUV420P);
            av_free(&buffer);
            pthread_mutex_unlock(&pVideoMedia->mutex);
            return S_FUNCTION_CONTINUE;
        }

        sws_scale(swsContext,
                  pVideoFrame->data,
                  pVideoFrame->linesize,
                  0,
                  pVideoFrame->height,
                  pFrameYUV420P->data,
                  pFrameYUV420P->linesize
        );

        double diffTime = pVideoMedia->getFrameDiffTime(pAudioMedia, pVideoMedia->getCurrentPTSByAVFrame(pVideoFrame));

        double delayTime = pVideoMedia->getDelayRenderTime(diffTime);

        LOGD(FFMPEG_TAG, "softDecodeVideo diffTime %lf delayTime %lf", diffTime, delayTime);

        av_usleep(static_cast<unsigned int>(delayTime * 1000000));

        if (pJavaMethods != nullptr && pVideoMedia != nullptr && pVideoMedia->pCodecContext != nullptr) {
            pJavaMethods->onCallJavaRenderYUVFromThread(
                    pVideoMedia->pCodecContext->width,
                    pVideoMedia->pCodecContext->height,
                    pFrameYUV420P->data[0],
                    pFrameYUV420P->data[1],
                    pFrameYUV420P->data[2]
            );
        }

        av_frame_free(&pFrameYUV420P);
        av_free(pFrameYUV420P);
        av_free(buffer);
        sws_freeContext(swsContext);
    }

    releaseFrame(&pVideoFrame);
    releasePacket(&pVideoPacket);
    pthread_mutex_unlock(&pVideoMedia->mutex);

    return S_SUCCESS;
}

int SFFmpeg::decodeAudio() {

    pAudioPacket = av_packet_alloc();

    if (pAudioPacket == nullptr) {
        LOGE(FFMPEG_TAG, "blockResampleAudio: failed to allocated memory for AVPacket");
        return S_FUNCTION_BREAK;
    }

    if (pAudioMedia == nullptr) {
        LOGE(FFMPEG_TAG, "blockResampleAudio: audio is null");
        return S_FUNCTION_BREAK;
    }

    if (getAudioAvPacketFromAudioQueue(pAudioPacket) == S_ERROR) {
        releasePacket(&pAudioPacket);
        releaseFrame(&pAudioFrame);
        return S_FUNCTION_CONTINUE;
    }

    pthread_mutex_lock(&pAudioMedia->mutex);

    int result = avcodec_send_packet(pAudioMedia->pCodecContext, pAudioPacket);

    if (result != S_SUCCESS) {
        releasePacket(&pVideoPacket);
        releaseFrame(&pVideoFrame);
        pthread_mutex_unlock(&pAudioMedia->mutex);
        LOGE(FFMPEG_TAG, "blockResampleAudio: send packet failed");
        return S_FUNCTION_CONTINUE;
    }

    pAudioFrame = av_frame_alloc();
    if (pAudioFrame == nullptr) {
        releasePacket(&pVideoPacket);
        releaseFrame(&pVideoFrame);
        pthread_mutex_unlock(&pAudioMedia->mutex);
        LOGE(FFMPEG_TAG, "blockResampleAudio: ailed to allocated memory for AVFrame");
        return S_FUNCTION_BREAK;
    }

    result = avcodec_receive_frame(pAudioMedia->pCodecContext, pAudioFrame);

    if (result != S_SUCCESS) {
        releasePacket(&pAudioPacket);
        releaseFrame(&pAudioFrame);
        pthread_mutex_unlock(&pAudioMedia->mutex);
        LOGE(FFMPEG_TAG, "blockResampleAudio: receive frame failed %d", result);
        return S_FUNCTION_CONTINUE;
    } else {

        // Process exception
        if (pAudioFrame->channels > 0 && pAudioFrame->channel_layout == 0) {
            pAudioFrame->channel_layout = (uint64_t) (av_get_default_channel_layout(
                    pAudioFrame->channels));
        } else if (pAudioFrame->channels == 0 && pAudioFrame->channel_layout > 0) {
            pAudioFrame->channels = av_get_channel_layout_nb_channels(pAudioFrame->channel_layout);
        }

        SwrContext *swrContext = swr_alloc_set_opts(NULL,
                                                    AV_CH_LAYOUT_STEREO,
                                                    AV_SAMPLE_FMT_S16,
                                                    pAudioFrame->sample_rate,
                                                    pAudioFrame->channel_layout,
                                                    (AVSampleFormat) (pAudioFrame->format),
                                                    pAudioFrame->sample_rate,
                                                    NULL, NULL);

        if (swrContext == nullptr || swr_init(swrContext) < 0) {

            releasePacket(&pAudioPacket);
            releaseFrame(&pAudioFrame);

            if (swrContext != nullptr) {
                swr_free(&swrContext);
                swrContext = nullptr;
            }

            LOGE(FFMPEG_TAG, "blockResampleAudio: swrContext is null or swrContext init failed");

            return S_FUNCTION_CONTINUE;
        }

        // number of samples output per channel, negative value on error
        channelSampleNumbers = swr_convert(swrContext,
                                           &pBuffer,
                                           pAudioFrame->nb_samples,
                                           (const uint8_t **) (pAudioFrame->data),
                                           pAudioFrame->nb_samples);

        if (channelSampleNumbers < 0) {
            LOGE(FFMPEG_TAG, "blockResampleAudio: swr convert numbers < 0 ");
            return S_FUNCTION_CONTINUE;
        }

        int outChannels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);

        int dataSize = channelSampleNumbers * outChannels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

        pAudioMedia->updateTime(pAudioFrame, dataSize);

        // Call Loading
        if (isLoading) {
            isLoading = false;
            if (pJavaMethods != nullptr) {
                pJavaMethods->onCallJavaLoadState(false);
            }
        }

        // Call Time
        if (pJavaMethods != nullptr && pAudioMedia->isMinDiff()) {
            pJavaMethods->onCallJavaTimeFromThread((int) pAudioMedia->getTotalTimeMillis(),
                                                   (int) pAudioMedia->getCurrentTimeMillis());
        }

        releasePacket(&pAudioPacket);
        releaseFrame(&pAudioFrame);

        swr_free(&swrContext);
        swrContext = nullptr;

        pthread_mutex_unlock(&pAudioMedia->mutex);
        return dataSize;
    }
}

SMedia *SFFmpeg::getAudioMedia() {
    return this->pAudioMedia;
}

SMedia *SFFmpeg::getVideoMedia() {
    return this->pVideoMedia;
}

SQueue *SFFmpeg::getAudioQueue() {
    return pAudioQueue;
}

SQueue *SFFmpeg::getVideoQueue() {
    return pVideoQueue;
}

int SFFmpeg::getAudioAvPacketFromAudioQueue(AVPacket *pPacket) {
    if (pAudioQueue != nullptr && pStatus != nullptr) {
        pAudioQueue->threadLock();
        while (pStatus->isLeastActiveState(STATE_PRE_PLAY)) {
            int result = pAudioQueue->getAvPacket(pPacket);
            if (result == S_FUNCTION_CONTINUE) {
                continue;
            } else if (result == S_SUCCESS) {
                break;
            }
        }
        pAudioQueue->threadUnlock();
        return S_SUCCESS;
    }
    return S_ERROR;
}

int SFFmpeg::getVideoAvPacketFromAudioQueue(AVPacket *pPacket) {
    if (pVideoQueue != nullptr && pStatus != nullptr) {
        pVideoQueue->threadLock();
        while (pStatus->isLeastActiveState(STATE_PRE_PLAY)) {
            int result = pVideoQueue->getAvPacket(pPacket);
            if (result == S_FUNCTION_CONTINUE) {
                continue;
            } else if (result == S_SUCCESS) {
                break;
            }
        }
        pVideoQueue->threadUnlock();
        return S_SUCCESS;
    }
    return S_ERROR;
}


uint8_t *SFFmpeg::getBuffer() {
    return pBuffer;
}

int SFFmpeg::stop() {
    if (pAudioQueue != nullptr) {
        pAudioQueue->clear();
    }
    if (pVideoQueue != nullptr) {
        pVideoQueue->clear();
    }
    return S_SUCCESS;
}

int SFFmpeg::release() {
    LOGD(FFMPEG_TAG, "close");

    if (pAudioMedia != nullptr) {
        if (pAudioMedia->pCodecContext != nullptr) {
            avcodec_close(pAudioMedia->pCodecContext);
            avcodec_free_context(&pAudioMedia->pCodecContext);
        }
        pAudioMedia = nullptr;
    }

    if (pVideoMedia != nullptr) {
        if (pVideoMedia->pCodecContext != nullptr) {
            avcodec_close(pVideoMedia->pCodecContext);
            avcodec_free_context(&pVideoMedia->pCodecContext);
        }
        pVideoMedia = nullptr;
    }

    if (pAudioQueue != nullptr) {
        pAudioQueue->clear();
    }

    if (pVideoQueue != nullptr) {
        pVideoQueue->clear();
    }

    if (pFormatContext != nullptr) {
        avformat_close_input(&pFormatContext);
        avformat_free_context(pFormatContext);
        pFormatContext = nullptr;
    }

    if (pBuffer != nullptr) {
        av_free(pBuffer);
        pBuffer = nullptr;
    }

    return S_SUCCESS;
}

double SFFmpeg::getTotalTimeMillis() {
    if (pAudioMedia != nullptr) {
        return pAudioMedia->getTotalTimeMillis();
    }
    return 0;
}

double SFFmpeg::getCurrentTimeMillis() {
    if (pAudioMedia != nullptr) {
        return pAudioMedia->getCurrentTimeMillis();
    }
    return 0;
}

void SFFmpeg::seek(int64_t millis) {

    pStatus->moveStatusToSeek();

    pthread_mutex_lock(&seekMutex);

    int64_t rel = millis / 1000 * AV_TIME_BASE;

    int ret = avformat_seek_file(pFormatContext, -1, INT64_MIN, rel, INT64_MAX, 0);

    if (pAudioMedia != nullptr) {
        if (pAudioQueue != nullptr) {
            pAudioQueue->clear();
        }
        pAudioMedia->currentTime = 0;
        pAudioMedia->currentRealTime = 0;
        pAudioMedia->currentTimeMillis = 0;
        pAudioMedia->lastTimeMillis = 0;
        pAudioMedia->clearCodecContextBuffer();
    }

    if (pVideoMedia != nullptr) {
        if (pVideoQueue != nullptr) {
            pVideoQueue->clear();
        }
        pVideoMedia->currentTime = 0;
        pVideoMedia->currentRealTime = 0;
        pVideoMedia->currentTimeMillis = 0;
        pVideoMedia->lastTimeMillis = 0;
        pVideoMedia->clearCodecContextBuffer();
    }

    pthread_mutex_unlock(&seekMutex);
    pStatus->moveStatusToPreState();

    LOGD(FFMPEG_TAG, "seek: seekTargetMillis=%d seekStartMillis=%f rel=%d ret=%d", (int) millis,
         pAudioMedia->currentTimeMillis,
         (int) rel, ret);
}

int SFFmpeg::getChannelSampleNumbers() const {
    return channelSampleNumbers;
}

void SFFmpeg::sleep() {
    av_usleep(1000 * 100);
}


void SFFmpeg::releaseFrame(AVFrame **avFrame) {
    if (avFrame != nullptr && (*avFrame) != nullptr) {
        av_frame_free(&(*avFrame));
        av_free(*avFrame);
        *avFrame = nullptr;
    }
}

void SFFmpeg::releasePacket(AVPacket **avPacket) {
    if (avPacket != nullptr && (*avPacket) != nullptr) {
        av_packet_free(&(*avPacket));
        av_free(*avPacket);
        *avPacket = nullptr;
    }
}


void SFFmpeg::startSoftDecode() {
    LOGD(FFMPEG_TAG, "SFFmpeg:startSoftDecode");
    while (pStatus->isLeastActiveState(STATE_PLAY)) {
        if (pStatus->isPause()) {
            sleep();
            continue;
        }
        int result = softDecodeVideo();
        if (result == S_FUNCTION_BREAK) {
            while (pStatus->isLeastActiveState(STATE_PLAY)) {
                SQueue *pAudioQueue = getAudioQueue();
                SQueue *pVideoQueue = getVideoQueue();
                if ((pAudioQueue != nullptr && pAudioQueue->getSize() > 0) ||
                    (pVideoQueue != nullptr && pVideoQueue->getSize() > 0)) {
                    sleep();
                    continue;
                } else {
                    pStatus->moveStatusToPreComplete();
                    break;
                }
            }
        }
    }
}

// ./configure --list-bsfs
//aac_adtstoasc		 extract_extradata	  hevc_metadata		   mp3_header_decompress    remove_extradata	     vp9_superframe
//av1_metadata		 filter_units		  hevc_mp4toannexb	   mpeg2_metadata	    text2movsub		     vp9_superframe_split
//chomp			 h264_metadata		  imx_dump_header	   mpeg4_unpack_bframes	    trace_headers
//dca_core		 h264_mp4toannexb	  mjpeg2jpeg		   noise		    truehd_core
//dump_extradata		 h264_redundant_pps	  mjpega_dump_header	   null			    vp9_metadata
//eac3_core		 hapqa_extract		  mov2textsub		   prores_metadata	    vp9_raw_reorder
void SFFmpeg::startMediaDecode() {
    LOGD(FFMPEG_TAG, "startMediaDecode: start");
    const char *codecName = pVideoMedia->pCodecContext->codec->name;
    if (strcasecmp(codecName, "h264") == 0) {
        pBSFilter = av_bsf_get_by_name("h264_mp4toannexb");
    }
    if (pBSFilter == nullptr) {
        LOGE(FFMPEG_TAG, "startMediaDecode: not found filter %s", codecName);
        startSoftDecode();
        return;
    }
    if (av_bsf_alloc(pBSFilter, &pVideoMedia->pABSContext) != 0) {
        LOGE(FFMPEG_TAG, "startMediaDecode: alloc abs fail %s", codecName);
        startSoftDecode();
        return;
    }
    if (avcodec_parameters_copy(pVideoMedia->pABSContext->par_in, pVideoMedia->pCodecParameters) < 0) {
        LOGE(FFMPEG_TAG, "startMediaDecode: copy params fail %s", codecName);
        av_bsf_free(&pVideoMedia->pABSContext);
        pVideoMedia->pABSContext = nullptr;
        startSoftDecode();
        return;
    }
    if (av_bsf_init(pVideoMedia->pABSContext) != 0) {
        LOGE(FFMPEG_TAG, "startMediaDecode: bsf init fail %s", codecName);
        av_bsf_free(&pVideoMedia->pABSContext);
        pVideoMedia->pABSContext = nullptr;
        startSoftDecode();
        return;
    }
    pVideoMedia->pABSContext->time_base_in = pVideoMedia->timeBase;

    pJavaMethods->onCallJavaInitMediaCodec(
            codecName,
            pVideoMedia->pCodecContext->width,
            pVideoMedia->pCodecContext->height,
            pVideoMedia->pCodecContext->extradata_size,
            pVideoMedia->pCodecContext->extradata,
            pVideoMedia->pCodecContext->extradata_size,
            pVideoMedia->pCodecContext->extradata
    );

    LOGD(FFMPEG_TAG, "SFFmpeg:startSoftDecode");
    while (pStatus->isLeastActiveState(STATE_PLAY)) {
        if (pStatus->isPause()) {
            sleep();
            continue;
        }
        int result = mediaDecodeVideo();
        if (result == S_FUNCTION_BREAK) {
            while (pStatus->isLeastActiveState(STATE_PLAY)) {
                SQueue *pAudioQueue = getAudioQueue();
                SQueue *pVideoQueue = getVideoQueue();
                if ((pAudioQueue != nullptr && pAudioQueue->getSize() > 0) ||
                    (pVideoQueue != nullptr && pVideoQueue->getSize() > 0)) {
                    sleep();
                    continue;
                } else {
                    pStatus->moveStatusToPreComplete();
                    break;
                }
            }
        }
    }
}

int SFFmpeg::mediaDecodeVideo() {
    if (pVideoMedia == nullptr || pVideoQueue == nullptr) {
        return S_FUNCTION_BREAK;
    }

    pVideoPacket = av_packet_alloc();

    if (pVideoPacket == nullptr) {
        LOGE(FFMPEG_TAG, "mediaDecodeVideo: failed to allocated memory for AVPacket");
        return S_FUNCTION_BREAK;
    }

    if (getVideoAvPacketFromAudioQueue(pVideoPacket) == S_ERROR) {
        releasePacket(&pVideoPacket);
        releaseFrame(&pVideoFrame);
        return S_FUNCTION_CONTINUE;
    }

    pthread_mutex_lock(&pVideoMedia->mutex);

    int result = av_bsf_send_packet(pVideoMedia->pABSContext, pVideoPacket);

    if (result != S_SUCCESS) {
        releasePacket(&pVideoPacket);
        releaseFrame(&pVideoFrame);
        pthread_mutex_unlock(&pVideoMedia->mutex);
        LOGE(FFMPEG_TAG, "mediaDecodeVideo: send packet failed");
        return S_FUNCTION_CONTINUE;
    }

    result = av_bsf_receive_packet(pVideoMedia->pABSContext, pVideoPacket);
    if (result != 0) {
        releasePacket(&pVideoPacket);
        releaseFrame(&pVideoFrame);
        pthread_mutex_unlock(&pVideoMedia->mutex);
        LOGE(FFMPEG_TAG, "mediaDecodeVideo: receive packet failed");
        return S_FUNCTION_CONTINUE;
    }

    double diffTime = pVideoMedia->getFrameDiffTime(pAudioMedia, pVideoMedia->getCurrentPTSByAVPacket(pVideoPacket));

    double delayTime = pVideoMedia->getDelayRenderTime(diffTime);

    LOGD(FFMPEG_TAG, "softDecodeVideo diffTime %lf delayTime %lf", diffTime, delayTime);

    av_usleep(static_cast<unsigned int>(delayTime * 1000000));

    pJavaMethods->onCallJavaMediaCodecDecodeAvPacket(pVideoPacket->size, pVideoPacket->data);

    releasePacket(&pVideoPacket);
    pthread_mutex_unlock(&pVideoMedia->mutex);

    return S_SUCCESS;
}
