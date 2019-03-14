#include "../include/SFFmpeg.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

SFFmpeg::SFFmpeg(SStatus *pStatus, SJavaMethods *pJavaMethods) {
    this->pStatus = pStatus;
    this->pJavaMethods = pJavaMethods;
    this->pAudioQueue = new SQueue();
    this->pVideoQueue = new SQueue();

    pthread_mutex_init(&seekMutex, NULL);
}

SFFmpeg::~SFFmpeg() {

    pthread_mutex_destroy(&seekMutex);

    delete pSource;
    pSource = NULL;

    if (pVideoQueue != NULL) {
        pVideoQueue->clear();
        delete pVideoQueue;
        pVideoQueue = NULL;
    }

    if (pAudioQueue != NULL) {
        pAudioQueue->clear();
        delete pAudioQueue;
        pAudioQueue = NULL;
    }

    release();

    delete pVideo;
    pVideo = NULL;

    delete pAudio;
    pAudio = NULL;

    pStatus = NULL;

    pJavaMethods = NULL;
}

void SFFmpeg::setSource(string *source) {
    if (source != NULL) {
        pSource = new string(source->c_str());
    }
}

int SFFmpeg::decodeMediaInfo() {

    pBuffer = (uint8_t *) (av_malloc(44100 * 2 * 2));

    // AVFormatContext holds the header information from the format (Container)
    // Allocating memory for this component
    // http://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
    pFormatContext = avformat_alloc_context();

    if (pFormatContext == NULL) {
        LOGE("SFFmpeg: decodeMediaInfo: ERROR could not allocate memory for Format Context");
        return S_ERROR;
    }

    if (pSource == NULL) {
        LOGE("SFFmpeg: decodeMediaInfo: ERROR source is empty");
        return S_ERROR;
    }

    LOGD("SFFmpeg: decodeMediaInfo: Opening the input file (%s) and _isLoading format (container) header",
         pSource->c_str());

    // Open the file and read its header. The codecs are not opened.
    // The function arguments are:
    // AVFormatContext (the component we allocated memory for),
    // url (filename),
    // AVInputFormat (if you pass NULL it'll do the auto detect)
    // and AVDictionary (which are options to the demuxer)
    // http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga31d601155e9035d5b0e7efedc894ee49
    int result = avformat_open_input(&pFormatContext, pSource->c_str(), NULL, NULL);
    if (result != 0) {
        LOGE("SFFmpeg: decodeMediaInfo: ERROR could not open the file %d", result);
        return S_ERROR;
    }

    // now we have access to some information about our file
    // since we read its header we can say what format (container) it's
    // and some other information related to the format itself.
    LOGD("SFFmpeg: decodeMediaInfo: Format %s, totalTime %lld us, bit_rate %lld", pFormatContext->iformat->name,
         pFormatContext->duration,
         pFormatContext->bit_rate);

    LOGD("SFFmpeg: decodeMediaInfo: Finding stream info from format");
    // read Packets from the Format to get stream information
    // this function populates pFormatContext->streams
    // (of size equals to pFormatContext->nb_streams)
    // the arguments are:
    // the AVFormatContext
    // and options contains options for codec corresponding to i-th stream.
    // On return each dictionary will be filled with options that were not found.
    // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#gad42172e27cddafb81096939783b157bb
    if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
        LOGD("SFFmpeg: decodeMediaInfo: ERROR could not get the stream info");
        return S_ERROR;
    }

    // loop though all the streams and print its main information
    for (int i = 0; i < pFormatContext->nb_streams; i++) {

        LOGD("SFFmpeg: decodeMediaInfo: ----------------------- ");

        AVCodecParameters *pLocalCodecParameters = NULL;
        pLocalCodecParameters = pFormatContext->streams[i]->codecpar;

        LOGD("SFFmpeg: decodeMediaInfo: AVStream->time_base before open coded %d/%d",
             pFormatContext->streams[i]->time_base.num,
             pFormatContext->streams[i]->time_base.den);

        LOGD("SFFmpeg: decodeMediaInfo: AVStream->r_frame_rate before open coded %d/%d",
             pFormatContext->streams[i]->r_frame_rate.num,
             pFormatContext->streams[i]->r_frame_rate.den);

        LOGD("SFFmpeg: decodeMediaInfo: AVStream->start_time %d", pFormatContext->streams[i]->start_time);
        LOGD("SFFmpeg: decodeMediaInfo: AVStream->totalTime %d", pFormatContext->streams[i]->duration);

        LOGD("SFFmpeg: decodeMediaInfo: Finding the proper decoder (CODEC)");

        // when the stream is a video we store its index, codec parameters and codec
        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {

            AVCodec *pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

            if (pLocalCodec == NULL) {
                LOGD("SFFmpeg: decodeMediaInfo: ERROR unsupported codec!");
                continue;
            }

            pVideo = new SMedia(i, pLocalCodec, pLocalCodecParameters);
            pVideo->totalTime = (pFormatContext->duration / AV_TIME_BASE);
            pAudio->totalTimeMillis = pAudio->totalTime * 1000;
            pVideo->timeBase = (pFormatContext->streams[pVideo->streamIndex]->time_base);

            LOGD("SFFmpeg: decodeMediaInfo: Video Codec: resolution %d x %d", pLocalCodecParameters->width,
                 pLocalCodecParameters->height);
            // print its name, id and bitrate
            LOGD("SFFmpeg: decodeMediaInfo: Codec %s ID %d bit_rate %lld", pLocalCodec->name, pLocalCodec->id,
                 pLocalCodecParameters->bit_rate);

        } else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {

            AVCodec *pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

            if (pLocalCodec == NULL) {
                LOGD("SFFmpeg: decodeMediaInfo: ERROR unsupported codec!");
                continue;
            }

            pAudio = new SMedia(i, pLocalCodec, pLocalCodecParameters);
            pAudio->totalTime = ((long) (pFormatContext->duration / AV_TIME_BASE));
            pAudio->totalTimeMillis = pAudio->totalTime * 1000;
            pAudio->timeBase = (pFormatContext->streams[pAudio->streamIndex]->time_base);

            LOGD("SFFmpeg: decodeMediaInfo: Audio Codec: %d channels, sample rate %d", pLocalCodecParameters->channels,
                 pLocalCodecParameters->sample_rate);
            LOGD("SFFmpeg: decodeMediaInfo: Codec %s ID %d bit_rate %lld", pLocalCodec->name, pLocalCodec->id,
                 pLocalCodecParameters->bit_rate);
        }
    }

    LOGD("SFFmpeg: decodeMediaInfo: ----------------------- ");

    if (pAudio != NULL) {
        // https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
        pAudio->pCodecContext = (avcodec_alloc_context3(pAudio->pCodec));
        if (pAudio->pCodecContext == NULL) {
            LOGE("SFFmpeg: decodeMediaInfo: Failed to allocated memory for AVCodecContext");
            return S_ERROR;
        }

        // Fill the codec context based on the values from the supplied codec parameters
        // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
        if (avcodec_parameters_to_context(pAudio->pCodecContext, pAudio->pCodecParameters) < 0) {
            LOGE("SFFmpeg: decodeMediaInfo: Failed to copy codec params to codec context");
            return S_ERROR;
        }

        // Initialize the AVCodecContext to use the given AVCodec.
        // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
        if (avcodec_open2(pAudio->pCodecContext, pAudio->pCodec, NULL) < 0) {
            LOGE("SFFmpeg: decodeMediaInfo: Failed to open codec through avcodec_open2");
            return S_ERROR;
        }
    }

    return S_SUCCESS;
}


int SFFmpeg::decodeFrame() {

    if (pStatus != NULL && pStatus->isSeek()) {
        sleep();
        return S_FUNCTION_CONTINUE;
    }

    if (pVideo == NULL && pAudio == NULL) {
        return S_ERROR;
    }

    if ((pAudio != NULL && pAudioQueue != NULL && pAudioQueue->getSize() > 40) ||
        (pVideo != NULL && pVideoQueue != NULL && pVideoQueue->getSize() > 40)) {
        sleep();
        return S_FUNCTION_CONTINUE;
    }

    // https://ffmpeg.org/doxygen/trunk/structAVPacket.html
    pDecodePacket = av_packet_alloc();
    if (pDecodePacket == NULL) {
        LOGE("SFFmpeg: decodeFrame: failed to allocated memory for AVPacket");
        return S_FUNCTION_BREAK;
    }

    pthread_mutex_lock(&seekMutex);
    int ret = av_read_frame(pFormatContext, pDecodePacket);
    pthread_mutex_unlock(&seekMutex);

    if (ret == 0) {
        if (pAudio != NULL && pDecodePacket->stream_index == pAudio->streamIndex) {
            pAudioQueue->putAvPacket(pDecodePacket);
        } else if (pVideo != NULL && pDecodePacket->stream_index == pVideo->streamIndex) {
            pVideoQueue->putAvPacket(pDecodePacket);
        }
        return S_SUCCESS;
    } else {
        av_packet_free(&pDecodePacket);
        av_free(pDecodePacket);
        // LOGD("SFFmpeg: decodeFrame: decode finished");
        return S_FUNCTION_BREAK;
    }
}

int SFFmpeg::resampleAudio() {

    pAudioResamplePacket = av_packet_alloc();

    if (pAudioResamplePacket == NULL) {
        LOGE("SFFmpeg: blockResampleAudio: failed to allocated memory for AVPacket");
        return S_FUNCTION_BREAK;
    }

    if (pAudio == NULL) {
        LOGE("SFFmpeg: blockResampleAudio: audio is null");
        return S_FUNCTION_BREAK;
    }

    if (getAudioAvPacketFromAudioQueue(pAudioResamplePacket) == S_ERROR) {

        releasePacket();
        releaseFrame();

        return S_FUNCTION_CONTINUE;
    }

    int result = avcodec_send_packet(pAudio->pCodecContext, pAudioResamplePacket);
    if (result != S_SUCCESS) {
        LOGE("SFFmpeg: blockResampleAudio: send packet failed");
        return S_FUNCTION_BREAK;
    }

    pAudioResampleFrame = av_frame_alloc();
    if (pAudioResampleFrame == NULL) {
        LOGE("SFFmpeg: blockResampleAudio: ailed to allocated memory for AVFrame");
        return S_FUNCTION_BREAK;
    }

    result = avcodec_receive_frame(pAudio->pCodecContext, pAudioResampleFrame);

    if (result != S_SUCCESS) {

        releasePacket();
        releaseFrame();

        LOGE("SFFmpeg: blockResampleAudio: receive frame failed");

        return S_FUNCTION_CONTINUE;

    } else {

        // Process exception
        if (pAudioResampleFrame->channels > 0 && pAudioResampleFrame->channel_layout == 0) {
            pAudioResampleFrame->channel_layout = (uint64_t) (av_get_default_channel_layout(
                    pAudioResampleFrame->channels));
        } else if (pAudioResampleFrame->channels == 0 && pAudioResampleFrame->channel_layout > 0) {
            pAudioResampleFrame->channels = av_get_channel_layout_nb_channels(pAudioResampleFrame->channel_layout);
        }

        SwrContext *swrContext = swr_alloc_set_opts(NULL,
                                                    AV_CH_LAYOUT_STEREO,
                                                    AV_SAMPLE_FMT_S16,
                                                    pAudioResampleFrame->sample_rate,
                                                    pAudioResampleFrame->channel_layout,
                                                    (AVSampleFormat) (pAudioResampleFrame->format),
                                                    pAudioResampleFrame->sample_rate,
                                                    NULL, NULL);

        if (swrContext == NULL || swr_init(swrContext) < 0) {

            releasePacket();
            releaseFrame();

            if (swrContext != NULL) {
                swr_free(&swrContext);
                swrContext = NULL;
            }

            LOGE("SFFmpeg: blockResampleAudio: swrContext is null or swrContext init failed");

            return S_FUNCTION_CONTINUE;
        }

        // number of samples output per channel, negative value on error
        channelSampleNumbers = swr_convert(swrContext,
                                           &pBuffer,
                                           pAudioResampleFrame->nb_samples,
                                           (const uint8_t **) (pAudioResampleFrame->data),
                                           pAudioResampleFrame->nb_samples);

        if (channelSampleNumbers < 0) {
            LOGE("SFFmpeg: blockResampleAudio: swr convert numbers < 0 ");
            return S_FUNCTION_CONTINUE;
        }

        int outChannels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);

        int dataSize = channelSampleNumbers * outChannels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

        pAudio->updateTime(pAudioResampleFrame, dataSize);

        releasePacket();
        releaseFrame();

        swr_free(&swrContext);
        swrContext = NULL;

        // Process Seek Exception
        if (pStatus != NULL && pStatus->isPrePreSeekState()) {
            if (seekTargetMillis < seekStartMillis) {
                // Left
                if (seekTargetMillis < pAudio->getCurrentTimeMillis()) {
                    LOGE("SFFmpeg: blockResampleAudio: left: frame error %d %d %d", pStatus->isPrePreSeekState(),
                         (int) seekTargetMillis,
                         (int) pAudio->getCurrentTimeMillis());
                    seek(seekTargetMillis);
                    if (seekTargetMillis > pAudio->getCurrentTimeMillis()) {

                        // Call Loading
                        if (!isLoading) {
                            isLoading = true;
                            if (pJavaMethods != NULL) {
                                pJavaMethods->onCallJavaLoadState(true);
                            }
                        }
                        return S_FUNCTION_CONTINUE;
                    }
                }
            } else if (seekTargetMillis > seekStartMillis) {
                // Right
                if (seekTargetMillis > pAudio->getCurrentTimeMillis()) {
                    seek(seekTargetMillis);
                    LOGE("SFFmpeg: blockResampleAudio: right: frame error %d %d %d", pStatus->isPrePreSeekState(),
                         (int) seekTargetMillis,
                         (int) pAudio->getCurrentTimeMillis());
                    if (seekTargetMillis > pAudio->getCurrentTimeMillis()) {

                        // Call Loading
                        if (!isLoading) {
                            isLoading = true;
                            if (pJavaMethods != NULL) {
                                pJavaMethods->onCallJavaLoadState(true);
                            }
                        }
                        return S_FUNCTION_CONTINUE;
                    }
                }
            }
        }

        seekTargetMillis = 0;
        seekStartMillis = 0;

        // Call Loading
        if (isLoading) {
            isLoading = false;
            if (pJavaMethods != NULL) {
                pJavaMethods->onCallJavaLoadState(false);
            }
        }

        // Call Time
        if (pJavaMethods != NULL && pAudio->isMinDiff()) {
            pJavaMethods->onCallJavaTimeFromThread((int) pAudio->getTotalTimeMillis(),
                                                   (int) pAudio->getCurrentTimeMillis());
        }
        return dataSize;
    }
}

void SFFmpeg::releaseFrame() {
    av_frame_free(&pAudioResampleFrame);
    av_free(pAudioResampleFrame);
    pAudioResampleFrame = NULL;
}

void SFFmpeg::releasePacket() {
    av_packet_free(&pAudioResamplePacket);
    av_free(pAudioResamplePacket);
    pAudioResamplePacket = NULL;
}

SMedia *SFFmpeg::getAudio() {
    return this->pAudio;
}

SMedia *SFFmpeg::getVideo() {
    return this->pVideo;
}

SQueue *SFFmpeg::getAudioQueue() {
    return pAudioQueue;
}

SQueue *SFFmpeg::getVideoQueue() {
    return pVideoQueue;
}

int SFFmpeg::getAudioAvPacketFromAudioQueue(AVPacket *pPacket) {
    if (pAudioQueue != NULL && pStatus != NULL) {
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

uint8_t *SFFmpeg::getBuffer() {
    return pBuffer;
}

int SFFmpeg::stop() {
    if (pAudioQueue != NULL) {
        pAudioQueue->clear();
    }
    if (pVideoQueue != NULL) {
        pVideoQueue->clear();
    }
    return S_SUCCESS;
}

int SFFmpeg::release() {
    LOGD("SFFmpeg: release");

    if (pAudio != NULL) {
        if (pAudio->pCodecContext != NULL) {
            avcodec_close(pAudio->pCodecContext);
            avcodec_free_context(&pAudio->pCodecContext);
        }
        pAudio = NULL;
    }

    if (pVideo != NULL) {
        if (pVideo->pCodecContext != NULL) {
            avcodec_close(pVideo->pCodecContext);
            avcodec_free_context(&pVideo->pCodecContext);
        }
        pVideo = NULL;
    }

    if (pAudioQueue != NULL) {
        pAudioQueue->clear();
    }

    if (pVideoQueue != NULL) {
        pVideoQueue->clear();
    }

    if (pFormatContext != NULL) {
        avformat_close_input(&pFormatContext);
        avformat_free_context(pFormatContext);
        pFormatContext = NULL;
    }

    if (pBuffer != NULL) {
        av_free(pBuffer);
        pBuffer = NULL;
    }

    return S_SUCCESS;
}

double SFFmpeg::getTotalTimeMillis() {
    if (pAudio != NULL) {
        return pAudio->getTotalTimeMillis();
    }
    return 0;
}

double SFFmpeg::getCurrentTimeMillis() {
    if (pAudio != NULL) {
        return pAudio->getCurrentTimeMillis();
    }
    return 0;
}

void SFFmpeg::seek(int64_t millis) {

    if (pAudioQueue != NULL) {
        pAudioQueue->clear();
    }
    if (pAudio != NULL) {
        pthread_mutex_lock(&seekMutex);
        pStatus->moveStatusToSeek();

        this->seekTargetMillis = millis;
        this->seekStartMillis = pAudio->currentTimeMillis;

        pAudio->currentTime = 0;
        pAudio->currentTimeMillis = 0;
        pAudio->lastTimeMillis = 0;

        int64_t rel = millis / 1000 * AV_TIME_BASE;
        int ret = avformat_seek_file(pFormatContext, -1, INT64_MIN, rel, INT64_MAX, 0);
        pStatus->moveStatusToPreState();

        pthread_mutex_unlock(&seekMutex);

        LOGD("SFFmpeg:seek: seekTargetMillis=%d seekStartMillis=%f rel=%d ret=%d", (int) seekTargetMillis,
             seekStartMillis,
             (int) rel, ret);
    }
}

int SFFmpeg::getChannelSampleNumbers() const {
    return channelSampleNumbers;
}

void SFFmpeg::sleep() {
    av_usleep(1000 * 100);
}

#pragma clang diagnostic pop