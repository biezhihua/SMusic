//
// Created by biezhihua on 2019/2/4.
//

#include "SFFmpeg.h"


SFFmpeg::SFFmpeg() {
    pBuffer = static_cast<uint8_t *>(av_malloc(44100 * 2 * 2));
}

SFFmpeg::~SFFmpeg() {

    delete pSource;
    pSource = NULL;

    delete pVideo;
    pVideo = NULL;

    delete pAudio;
    pAudio = NULL;

    av_free(pBuffer);
    pBuffer = NULL;

    avformat_free_context(pFormatContext);
    pFormatContext = NULL;
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
        LOGE("ERROR could not allocate memory for Format Context");
        return -1;
    }

    if (pSource == NULL) {
        LOGE("ERROR source is empty");
        return -1;
    }

    LOGD("opening the input file (%s) and loading format (container) header", pSource->c_str());

    // Open the file and read its header. The codecs are not opened.
    // The function arguments are:
    // AVFormatContext (the component we allocated memory for),
    // url (filename),
    // AVInputFormat (if you pass NULL it'll do the auto detect)
    // and AVDictionary (which are options to the demuxer)
    // http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga31d601155e9035d5b0e7efedc894ee49
    int result = avformat_open_input(&pFormatContext, pSource->c_str(), NULL, NULL);
    if (result != 0) {
        LOGE("ERROR could not open the file %d", result);
        return -1;
    }

    // now we have access to some information about our file
    // since we read its header we can say what format (container) it's
    // and some other information related to the format itself.
    LOGD("format %s, duration %lld us, bit_rate %lld", pFormatContext->iformat->name, pFormatContext->duration,
         pFormatContext->bit_rate);

    LOGD("finding stream info from format");
    // read Packets from the Format to get stream information
    // this function populates pFormatContext->streams
    // (of size equals to pFormatContext->nb_streams)
    // the arguments are:
    // the AVFormatContext
    // and options contains options for codec corresponding to i-th stream.
    // On return each dictionary will be filled with options that were not found.
    // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#gad42172e27cddafb81096939783b157bb
    if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
        LOGD("ERROR could not get the stream info");
        return -1;
    }

    // loop though all the streams and print its main information
    for (int i = 0; i < pFormatContext->nb_streams; i++) {

        AVCodecParameters *pLocalCodecParameters = NULL;
        pLocalCodecParameters = pFormatContext->streams[i]->codecpar;

        LOGD("AVStream->time_base before open coded %d/%d",
             pFormatContext->streams[i]->time_base.num,
             pFormatContext->streams[i]->time_base.den);

        LOGD("AVStream->r_frame_rate before open coded %d/%d",
             pFormatContext->streams[i]->r_frame_rate.num,
             pFormatContext->streams[i]->r_frame_rate.den);

        LOGD("AVStream->start_time %"
                     PRId64, pFormatContext->streams[i]->start_time);
        LOGD("AVStream->duration %"
                     PRId64, pFormatContext->streams[i]->duration);

        LOGD("finding the proper decoder (CODEC)");

        // when the stream is a video we store its index, codec parameters and codec
        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {

            AVCodec *pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

            if (pLocalCodec == NULL) {
                LOGE("ERROR unsupported codec!");
                continue;
            }


            pVideo = new SMedia(i, pLocalCodec, pLocalCodecParameters);

            LOGD("Video Codec: resolution %d x %d", pLocalCodecParameters->width, pLocalCodecParameters->height);
            // print its name, id and bitrate
            LOGD("\tCodec %s ID %d bit_rate %lld", pLocalCodec->name, pLocalCodec->id, pLocalCodecParameters->bit_rate);
        } else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {

            AVCodec *pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

            if (pLocalCodec == NULL) {
                LOGE("ERROR unsupported codec!");
                continue;
            }

            pAudio = new SMedia(i, pLocalCodec, pLocalCodecParameters);

            LOGD("Audio Codec: %d channels, sample rate %d", pLocalCodecParameters->channels,
                 pLocalCodecParameters->sample_rate);
            LOGD("\tCodec %s ID %d bit_rate %lld", pLocalCodec->name, pLocalCodec->id, pLocalCodecParameters->bit_rate);
        }
    }

    if (pAudio != NULL) {
        // https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
        pAudio->setCodecContext(avcodec_alloc_context3(pAudio->getCodec()));
        if (pAudio->getCodecContext() == NULL) {
            LOGE("failed to allocated memory for AVCodecContext");
            return -1;
        }

        // Fill the codec context based on the values from the supplied codec parameters
        // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
        if (avcodec_parameters_to_context(pAudio->getCodecContext(), pAudio->getCodecParameters()) < 0) {
            LOGE("failed to copy codec params to codec context");
            return -1;
        }

        // Initialize the AVCodecContext to use the given AVCodec.
        // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
        if (avcodec_open2(pAudio->getCodecContext(), pAudio->getCodec(), NULL) < 0) {
            LOGE("failed to open codec through avcodec_open2");
            return -1;
        }
    }

    return 0;
}

int count = 0;

int SFFmpeg::decodeAudioFrame() {
    // https://ffmpeg.org/doxygen/trunk/structAVPacket.html
    pDecodePacket = av_packet_alloc();
    if (!pDecodePacket) {
        LOGE("failed to allocated memory for AVPacket");
        return ERROR_BREAK;
    }
    if (av_read_frame(pFormatContext, pDecodePacket) == 0) {
        if (pDecodePacket->stream_index == pAudio->getStreamIndex()) {
            count++;
            LOGD("fill the Packet with data from the Stream %d", count);
            pAudio->putAvPacketToQueue(pDecodePacket);
        } else {
            LOGD("other stream index");
        }
    } else {
        av_packet_free(&pDecodePacket);
        av_free(pDecodePacket);
        LOGD("decode finished");
        return ERROR_BREAK;
    }

    return SUCCESS;
}

int SFFmpeg::resampleAudio() {

    pResamplePacket = av_packet_alloc();
    if (!pResamplePacket) {
        LOGE("failed to allocated memory for AVPacket");
        return ERROR_BREAK;
    }

    if (pAudio == NULL) {
        LOGE("audio is null");
        return ERROR_BREAK;
    }

    if (pAudio->getAvPacketFromQueue(pResamplePacket) != 0) {
        av_packet_free(&pResamplePacket);
        av_free(pResamplePacket);
        pResamplePacket = NULL;
        return ERROR_CONTINUE;
    }

    int result = avcodec_send_packet(pAudio->getCodecContext(), pResamplePacket);

    if (!result) {
        return ERROR_BREAK;
    }

    pResampleFrame = av_frame_alloc();
    result = avcodec_receive_frame(pAudio->getCodecContext(), pResampleFrame);
    if (!result) {
        av_packet_free(&pResamplePacket);
        av_free(pResamplePacket);
        pResamplePacket = NULL;
        av_frame_free(&pResampleFrame);
        av_free(pResampleFrame);
        pResampleFrame = NULL;
        return ERROR_CONTINUE;
    } else {

        // Process exception
        if (pResampleFrame->channels > 0 && pResampleFrame->channel_layout == 0) {
            pResampleFrame->channel_layout = (uint64_t) (av_get_default_channel_layout(
                    pResampleFrame->channels));
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
            av_packet_free(&pResamplePacket);
            av_free(pResamplePacket);
            pResamplePacket = NULL;
            av_frame_free(&pResampleFrame);
            av_free(pResampleFrame);
            pResampleFrame = NULL;
            if (swrContext != NULL) {
                swr_free(&swrContext);
                swrContext = NULL;
            }
            return ERROR_CONTINUE;
        }

        int numbers = swr_convert(swrContext,
                                  &pBuffer,
                                  pResampleFrame->nb_samples,
                                  (const uint8_t **) (pResampleFrame->data),
                                  pResampleFrame->nb_samples);

        if (numbers < 0) {
            return ERROR_CONTINUE;
        }

        int outChannels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);

        int dataSize = numbers * outChannels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

        av_packet_free(&pResamplePacket);
        av_free(pResamplePacket);
        pResamplePacket = NULL;
        av_frame_free(&pResampleFrame);
        av_free(pResampleFrame);
        pResampleFrame = NULL;
        swr_free(&swrContext);
        swrContext = NULL;

        return dataSize;
    }

    return SUCCESS;
}

SMedia *SFFmpeg::getAudio() {
    return this->pAudio;
}

SMedia *SFFmpeg::getVideo() {
    return this->pVideo;
}
