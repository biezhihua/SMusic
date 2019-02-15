//
// Created by biezhihua on 2019/2/4.
//

#include "SFFmpeg.h"


SFFmpeg::SFFmpeg(SStatus *pStatus) {
    this->pStatus = pStatus;
    pBuffer = static_cast<uint8_t *>(av_malloc(44100 * 2 * 2));
    pAudioQueue = new SQueue();
    pVideoQueue = new SQueue();
}

SFFmpeg::~SFFmpeg() {

    delete pSource;
    pSource = NULL;

    delete pVideo;
    pVideo = NULL;

    delete pVideoQueue;
    pVideoQueue = NULL;

    delete pAudioQueue;
    pAudioQueue = NULL;

    delete pAudio;
    pAudio = NULL;

    av_free(pBuffer);
    pBuffer = NULL;

    avformat_free_context(pFormatContext);
    pFormatContext = NULL;

    pStatus = NULL;
}

void SFFmpeg::setSource(string *source) {
    if (source != NULL) {
        pSource = new string(source->c_str());
    }
}

int SFFmpeg::decodeMediaInfo() {

    // AVFormatContext holds the header information from the format (Container)
    // Allocating memory for this component
    // http://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
    pFormatContext = avformat_alloc_context();

    if (pFormatContext == NULL) {
        LOGE("SFFmpeg: decodeMediaInfo: ERROR could not allocate memory for Format Context");
        return S_ERROR_INVALID;
    }

    if (pSource == NULL) {
        LOGE("SFFmpeg: decodeMediaInfo: ERROR source is empty");
        return S_ERROR_INVALID;
    }

    LOGD("SFFmpeg: decodeMediaInfo: Opening the input file (%s) and loading format (container) header",
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
        return S_ERROR_INVALID;
    }

    // now we have access to some information about our file
    // since we read its header we can say what format (container) it's
    // and some other information related to the format itself.
    LOGD("SFFmpeg: decodeMediaInfo: Format %s, duration %lld us, bit_rate %lld", pFormatContext->iformat->name,
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
        return S_ERROR_INVALID;
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

        LOGD("SFFmpeg: decodeMediaInfo: AVStream->start_time %"
                     PRId64, pFormatContext->streams[i]->start_time);
        LOGD("SFFmpeg: decodeMediaInfo: AVStream->duration %"
                     PRId64, pFormatContext->streams[i]->duration);

        LOGD("SFFmpeg: decodeMediaInfo: Finding the proper decoder (CODEC)");

        // when the stream is a video we store its index, codec parameters and codec
        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {

            AVCodec *pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

            if (pLocalCodec == NULL) {
                LOGD("SFFmpeg: decodeMediaInfo: ERROR unsupported codec!");
                continue;
            }

            pVideo = new SMedia(i, pLocalCodec, pLocalCodecParameters);

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

            LOGD("SFFmpeg: decodeMediaInfo: Audio Codec: %d channels, sample rate %d", pLocalCodecParameters->channels,
                 pLocalCodecParameters->sample_rate);
            LOGD("SFFmpeg: decodeMediaInfo: Codec %s ID %d bit_rate %lld", pLocalCodec->name, pLocalCodec->id,
                 pLocalCodecParameters->bit_rate);
        }
    }

    LOGD("SFFmpeg: decodeMediaInfo: ----------------------- ");

    if (pAudio != NULL) {
        // https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
        pAudio->setCodecContext(avcodec_alloc_context3(pAudio->getCodec()));
        if (pAudio->getCodecContext() == NULL) {
            LOGE("SFFmpeg: decodeMediaInfo: Failed to allocated memory for AVCodecContext");
            return S_ERROR_INVALID;
        }

        // Fill the codec context based on the values from the supplied codec parameters
        // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
        if (avcodec_parameters_to_context(pAudio->getCodecContext(), pAudio->getCodecParameters()) < 0) {
            LOGE("SFFmpeg: decodeMediaInfo: Failed to copy codec params to codec context");
            return S_ERROR_INVALID;
        }

        // Initialize the AVCodecContext to use the given AVCodec.
        // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
        if (avcodec_open2(pAudio->getCodecContext(), pAudio->getCodec(), NULL) < 0) {
            LOGE("SFFmpeg: decodeMediaInfo: Failed to open codec through avcodec_open2");
            return S_ERROR_INVALID;
        }
    }

    return S_SUCCESS;
}

int count = 0;

int SFFmpeg::decodeAudioFrame() {
    // https://ffmpeg.org/doxygen/trunk/structAVPacket.html
    pDecodePacket = av_packet_alloc();
    if (!pDecodePacket) {
        LOGE("failed to allocated memory for AVPacket");
        return S_ERROR_BREAK;
    }
    if (av_read_frame(pFormatContext, pDecodePacket) == 0) {
        if (pDecodePacket->stream_index == pAudio->getStreamIndex()) {
            count++;
            LOGD("fill the Packet with data from the Stream %d", count);
            pAudioQueue->putAvPacket(pDecodePacket);
        } else {
            LOGD("other stream index");
        }
    } else {
        av_packet_free(&pDecodePacket);
        av_free(pDecodePacket);
        LOGD("decode finished");
        return S_ERROR_BREAK;
    }

    return S_SUCCESS;
}

int SFFmpeg::resampleAudio() {

    pResamplePacket = av_packet_alloc();

    if (!pResamplePacket) {
        LOGE("SSFFmpeg: resampleAudio: failed to allocated memory for AVPacket");
        return S_ERROR_BREAK;
    }

    if (pAudio == NULL) {
        LOGE("SSFFmpeg: resampleAudio: audio is null");
        return S_ERROR_BREAK;
    }

    if (getAvPacketFromQueue(pResamplePacket) != S_ERROR) {

        releasePacket();
        releaseFrame();

        return S_ERROR_CONTINUE;
    }

    int result = avcodec_send_packet(pAudio->getCodecContext(), pResamplePacket);
    if (!result) {
        LOGE("fSSFFmpeg: resampleAudio: send packet failed");
        return S_ERROR_CONTINUE;
    }

    pResampleFrame = av_frame_alloc();
    if (!pResampleFrame) {
        LOGE("fSSFFmpeg: resampleAudio: ailed to allocated memory for AVFrame");
        return S_ERROR_BREAK;
    }

    result = avcodec_receive_frame(pAudio->getCodecContext(), pResampleFrame);

    if (!result) {

        releasePacket();
        releaseFrame();

        LOGE("fSSFFmpeg: resampleAudio: receive frame failed");

        return S_ERROR_CONTINUE;

    } else {

        // Process exception
        if (pResampleFrame->channels > 0 && pResampleFrame->channel_layout == 0) {
            pResampleFrame->channel_layout = (uint64_t) (av_get_default_channel_layout(pResampleFrame->channels));
        } else if (pResampleFrame->channels == 0 && pResampleFrame->channel_layout > 0) {
            pResampleFrame->channels = av_get_channel_layout_nb_channels(pResampleFrame->channel_layout);
        }

        SwrContext *swrContext = swr_alloc_set_opts(NULL,
                                                    AV_CH_LAYOUT_STEREO,
                                                    AV_SAMPLE_FMT_S16,
                                                    pResampleFrame->sample_rate,
                                                    pResampleFrame->channel_layout,
                                                    (AVSampleFormat) (pResampleFrame->format),
                                                    pResampleFrame->sample_rate,
                                                    NULL, NULL);

        if (swrContext == NULL || swr_init(swrContext) < 0) {

            releasePacket();
            releaseFrame();

            if (swrContext != NULL) {
                swr_free(&swrContext);
                swrContext = NULL;
            }

            LOGE("fSSFFmpeg: resampleAudio: swrContext is null or swrContext init failed");

            return S_ERROR_CONTINUE;
        }

        int numbers = swr_convert(swrContext,
                                  &pBuffer,
                                  pResampleFrame->nb_samples,
                                  (const uint8_t **) (pResampleFrame->data),
                                  pResampleFrame->nb_samples);

        if (numbers < 0) {
            LOGE("fSSFFmpeg: resampleAudio: swr convert numbers < 0 ");
            return S_ERROR_CONTINUE;
        }

        int outChannels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);

        int dataSize = numbers * outChannels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

        releasePacket();
        releaseFrame();

        swr_free(&swrContext);
        swrContext = NULL;

        return dataSize;
    }

    return S_SUCCESS;
}

void SFFmpeg::releaseFrame() {
    av_frame_free(&pResampleFrame);
    av_free(pResampleFrame);
    pResampleFrame = NULL;
}

void SFFmpeg::releasePacket() {
    av_packet_free(&pResamplePacket);
    av_free(pResamplePacket);
    pResamplePacket = NULL;
}

SMedia *SFFmpeg::getAudio() {
    return this->pAudio;
}

SMedia *SFFmpeg::getVideo() {
    return this->pVideo;
}

SQueue *SFFmpeg::getAudioQueue() const {
    return pAudioQueue;
}

SQueue *SFFmpeg::getPVideoQueue() const {
    return pVideoQueue;
}

int SFFmpeg::getAvPacketFromQueue(AVPacket *pPacket) {
    if (pAudioQueue != NULL && pStatus != NULL) {
        pAudioQueue->threadLock();
        while (pStatus->isLeastActiveState(STATE_PRE_PLAY)) {
            int result = pAudioQueue->getAvPacket(pPacket);
            if (result == S_ERROR_CONTINUE) {
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
