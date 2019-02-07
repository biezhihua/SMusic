//
// Created by biezhihua on 2019/2/4.
//

#include "SFFmpeg.h"

SFFmpeg::SFFmpeg() {
}

SFFmpeg::~SFFmpeg() {
    pFormatContext = NULL;

    delete pSource;
    pSource = NULL;

    delete pVideo;
    pVideo = NULL;

    delete pAudio;
    pAudio = NULL;
}

void SFFmpeg::setSource(string *pSource) {
    this->pSource = new string(pSource->c_str());
}

int SFFmpeg::decodeMediaInfo() {

    av_register_all();

    avformat_network_init();

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

        AVCodec *pLocalCodec = NULL;

        // finds the registered decoder for a codec ID
        // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga19a0ca553277f019dd5b0fec6e1f9dca
        pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

        if (pLocalCodec == NULL) {
            LOGE("ERROR unsupported codec!");
            return -1;
        }

        // when the stream is a video we store its index, codec parameters and codec
        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {

            pVideo = new SMedia(i, pLocalCodec, pLocalCodecParameters);

            LOGD("Video Codec: resolution %d x %d", pLocalCodecParameters->width, pLocalCodecParameters->height);
            // print its name, id and bitrate
            LOGD("\tCodec %s ID %d bit_rate %lld", pLocalCodec->name, pLocalCodec->id, pLocalCodecParameters->bit_rate);
        } else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {

            pAudio = new SMedia(i, pLocalCodec, pLocalCodecParameters);

            LOGD("Audio Codec: %d channels, sample rate %d", pLocalCodecParameters->channels,
                 pLocalCodecParameters->sample_rate);
            LOGD("\tCodec %s ID %d bit_rate %lld", pLocalCodec->name, pLocalCodec->id, pLocalCodecParameters->bit_rate);
        }
    }

    if (pAudio != NULL) {
        // https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
        AVCodecContext *pCodecContext = avcodec_alloc_context3(pAudio->getCodec());
        if (pCodecContext == NULL) {
            LOGE("failed to allocated memory for AVCodecContext");
            return -1;
        }

        // Fill the codec context based on the values from the supplied codec parameters
        // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
        if (avcodec_parameters_to_context(pCodecContext, pAudio->getCodecParameters()) < 0) {
            LOGE("failed to copy codec params to codec context");
            return -1;
        }

        // Initialize the AVCodecContext to use the given AVCodec.
        // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
        if (avcodec_open2(pCodecContext, pAudio->getCodec(), NULL) < 0) {
            LOGE("failed to open codec through avcodec_open2");
            return -1;
        }
    }

    return 0;
}

int SFFmpeg::decodeAudioFrame() {

    // https://ffmpeg.org/doxygen/trunk/structAVPacket.html
    AVPacket *pPacket = av_packet_alloc();
    if (!pPacket) {
        LOGE("failed to allocated memory for AVPacket");
        return -1;
    }
    int count = 0;
    while (pAudio != NULL) {
        if (av_read_frame(pFormatContext, pPacket) == 0) {
            if (pPacket->stream_index == pAudio->getStreamIndex()) {
                count++;
                LOGD("fill the Packet with data from the Stream %d", count);
                pAudio->putAvPacketToQueue(pPacket);
            } else {
                av_packet_free(&pPacket);
                av_free(pPacket);
            }
        } else {
            LOGE("decode finished");
            av_packet_free(&pPacket);
            av_free(pPacket);
            break;
        }
    }

    return 0;
}
