#include "PlayerInfoStatus.h"

PlayerInfoStatus::PlayerInfoStatus() {
    init();
    reset();
}

PlayerInfoStatus::~PlayerInfoStatus() {
    reset();
    // 文件路径
    if (url) {
        av_freep(&url);
        url = nullptr;
    }
}

void PlayerInfoStatus::init() {
    swsOpts = (AVDictionary *) malloc(sizeof(AVDictionary));
    memset(swsOpts, 0, sizeof(AVDictionary));
    swrOpts = (AVDictionary *) malloc(sizeof(AVDictionary));
    memset(swrOpts, 0, sizeof(AVDictionary));
    formatOpts = (AVDictionary *) malloc(sizeof(AVDictionary));
    memset(formatOpts, 0, sizeof(AVDictionary));
    codecOpts = (AVDictionary *) malloc(sizeof(AVDictionary));
    memset(codecOpts, 0, sizeof(AVDictionary));

    inputFormat = nullptr;
    url = nullptr;
    headers = nullptr;
    videoTitle = nullptr;

    audioCodecName = nullptr;
    videoCodecName = nullptr;
}

void PlayerInfoStatus::reset() {
    if (DEBUG) {
        ALOGD(TAG, "[%s]", __func__);
    }

    mutex.lock();

    if (swsOpts) {
        av_dict_free(&swsOpts);
        av_dict_set(&swsOpts, "flags", "bicubic", 0);
    }

    if (swrOpts) {
        av_dict_free(&swrOpts);
    }

    if (formatOpts) {
        av_dict_free(&formatOpts);
    }

    if (codecOpts) {
        av_dict_free(&codecOpts);
    }

    offset = 0;

    abortRequest = 1;

    pauseRequest = 1;

    seekByBytes = 0;

    syncType = AV_SYNC_AUDIO;

    startTime = AV_NOPTS_VALUE;

    duration = AV_NOPTS_VALUE;

    durationSec = AV_NOPTS_VALUE;

    realTime = 0;

    infiniteBuffer = -1;

    audioDisable = 0;

    videoDisable = 0;

    displayDisable = 0;

    fast = 0;

    generateMissingPts = 0;

    lowResolution = 0;

    playbackRate = 1.0;

    playbackPitch = 1.0;

    seekRequest = 0;

    seekFlags = 0;

    seekPos = 0;

    seekRel = 0;

    autoExit = 1;

    loopTimes = 1;

    audioMute = 0;

    dropFrameWhenSlow = 1;

    decoderReorderPts = -1;

    eof = 0;

    attachmentRequest = 0;

    inputFormat = nullptr;

    formatContext = nullptr;

    videoTitle = nullptr;

    headers = nullptr;

    videoIndex = -1;

    audioIndex = -1;

    mutex.unlock();
}

void PlayerInfoStatus::setOption(int category, const char *type, const char *option) {
    if (DEBUG) {
        ALOGD("PlayerInfoStatus", "[%s] category=%d type=%s option=%s", __func__, category, type,
              option);
    }
    switch (category) {
        case OPT_CATEGORY_FORMAT: {
            av_dict_set(&formatOpts, type, option, 0);
            break;
        }

        case OPT_CATEGORY_CODEC: {
            av_dict_set(&codecOpts, type, option, 0);
            break;
        }

        case OPT_CATEGORY_SWS: {
            av_dict_set(&swsOpts, type, option, 0);
            break;
        }

        case OPT_CATEGORY_PLAYER: {
            parse_string(type, option);
            break;
        }

        case OPT_CATEGORY_SWR: {
            av_dict_set(&swrOpts, type, option, 0);
            break;
        }
    }
}

void PlayerInfoStatus::setOptionLong(int category, const char *type, int64_t option) {
    switch (category) {
        case OPT_CATEGORY_FORMAT: {
            av_dict_set_int(&formatOpts, type, option, 0);
            break;
        }

        case OPT_CATEGORY_CODEC: {
            av_dict_set_int(&codecOpts, type, option, 0);
            break;
        }

        case OPT_CATEGORY_SWS: {
            av_dict_set_int(&swsOpts, type, option, 0);
            break;
        }

        case OPT_CATEGORY_PLAYER: {
            parse_int(type, option);
            break;
        }

        case OPT_CATEGORY_SWR: {
            av_dict_set_int(&swrOpts, type, option, 0);
            break;
        }
    }
}

void PlayerInfoStatus::parse_string(const char *type, const char *option) {
    if (!strcmp("acodec", type)) { // 指定音频解码器名称
        audioCodecName = av_strdup(option);
    } else if (!strcmp("vcodec", type)) {   // 指定视频解码器名称
        videoCodecName = av_strdup(option);
    } else if (!strcmp("sync", type)) { // 制定同步类型
        if (!strcmp("audio", option)) {
            syncType = AV_SYNC_AUDIO;
        } else if (!strcmp("video", option)) {
            syncType = AV_SYNC_VIDEO;
        } else if (!strcmp("ext", option)) {
            syncType = AV_SYNC_EXTERNAL;
        } else {    // 其他则使用默认的音频同步
            syncType = AV_SYNC_AUDIO;
        }
    } else if (!strcmp("f", type)) { // f 指定输入文件格式
        inputFormat = av_find_input_format(option);
        if (!inputFormat) {
            ALOGD(TAG, "[%s] Unknown input format: %s", __func__, option);
        }
    }
}

void PlayerInfoStatus::parse_int(const char *type, int64_t option) {
    if (!strcmp("an", type)) { // 禁用音频
        audioDisable = (option != 0) ? 1 : 0;
    } else if (!strcmp("vn", type)) { // 禁用视频
        videoDisable = (option != 0) ? 1 : 0;
    } else if (!strcmp("bytes", type)) { // 以字节方式定位
        seekByBytes = (option > 0) ? 1 : ((option < 0) ? -1 : 0);
    } else if (!strcmp("nodisp", type)) { // 不显示
        displayDisable = (option != 0) ? 1 : 0;
    } else if (!strcmp("fast", type)) { // fast标志
        fast = (option != 0) ? 1 : 0;
    } else if (!strcmp("generateMissingPts", type)) { // genpts标志
        generateMissingPts = (option != 0) ? 1 : 0;
    } else if (!strcmp("lowResolution", type)) { // lowres标准字
        lowResolution = (option != 0) ? 1 : 0;
    } else if (!strcmp("drp", type)) { // 重排pts
        decoderReorderPts = (option != 0) ? 1 : 0;
    } else if (!strcmp("autoexit", type)) { // 自动退出标志
        autoExit = (option != 0) ? 1 : 0;
    } else if (!strcmp("framedrop", type)) { // 丢帧标志
        dropFrameWhenSlow = (option != 0) ? 1 : 0;
    } else if (!strcmp("infbuf", type)) { // 无限缓冲区标志
        infiniteBuffer = (option > 0) ? 1 : ((option < 0) ? -1 : 0);
    } else {
        ALOGE(TAG, "[%s] unknown option - '%s'", __func__, type);
    }
}

const char *PlayerInfoStatus::getSyncType() {
    if (syncType == AV_SYNC_AUDIO) {
        return "AV_SYNC_AUDIO";
    } else if (syncType == AV_SYNC_VIDEO) {
        return "AV_SYNC_VIDEO";
    } else if (syncType == AV_SYNC_EXTERNAL) {
        return "AV_SYNC_EXTERNAL";
    }
    return "NONE";
}

void PlayerInfoStatus::setFormatContext(AVFormatContext *formatContext) {
    PlayerInfoStatus::formatContext = formatContext;
}


void PlayerInfoStatus::setAbortRequest(int abortRequest) {
    mutex.lock();
    PlayerInfoStatus::abortRequest = abortRequest;
    mutex.unlock();
}

void PlayerInfoStatus::setPauseRequest(int pauseRequest) {
    mutex.lock();
    PlayerInfoStatus::pauseRequest = pauseRequest;
    mutex.unlock();
}

void PlayerInfoStatus::setSeekRequest(int seekRequest) {
    mutex.lock();
    PlayerInfoStatus::seekRequest = seekRequest;
    mutex.unlock();
}


