#include "Options.h"

void Options::showOptions() {
    ALOGD(OPTIONS_TAG, "===== versions =====");
    ALOGD(OPTIONS_TAG, "%-*s: %s", VERSION_MODULE_FILE_NAME_LENGTH, "FFmpeg", av_version_info());
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "libavutil", avutil_version());
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "libavcodec", avcodec_version());
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "libavformat", avformat_version());
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "libswscale", swscale_version());
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "libswresample", swresample_version());
    ALOGD(OPTIONS_TAG, "===== end =====");

    ALOGD(OPTIONS_TAG, "===== options =====");
    ALOGD(OPTIONS_TAG, "%-*s: %s", VERSION_MODULE_FILE_NAME_LENGTH, "inputFileName", inputFileName);
    ALOGD(OPTIONS_TAG, "%-*s: %s", VERSION_MODULE_FILE_NAME_LENGTH, "windowTitle", windowTitle);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "isFullScreen", isFullScreen);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "defaultWidth", defaultWidth);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "defaultHeight", defaultHeight);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "screenLeft", screenLeft);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "screenTop", screenTop);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "seekByBytes", seekByBytes);
    ALOGD(OPTIONS_TAG, "%-*s: %f", VERSION_MODULE_FILE_NAME_LENGTH, "seekInterval", seekInterval);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "startupVolume", startupVolume);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "syncType", syncType);
    ALOGD(OPTIONS_TAG, "%-*s: %lld", VERSION_MODULE_FILE_NAME_LENGTH, "startTime", startTime);
    ALOGD(OPTIONS_TAG, "%-*s: %lld", VERSION_MODULE_FILE_NAME_LENGTH, "duration", duration);
    // ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "borderLess", borderLess);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "fast", fast);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "generatePts", generatePts);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "lowres", lowres);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "autoRotate", autoRotate);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "decoderReorderPts", decoderReorderPts);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "autoExit", autoExit);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "loop", loop);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "dropFrameWhenSlow", dropFrameWhenSlow);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "showMode", showMode);
    ALOGD(OPTIONS_TAG, "%-*s: %s", VERSION_MODULE_FILE_NAME_LENGTH, "inputFormatName", inputFormatName);
    ALOGD(OPTIONS_TAG, "%-*s: %s", VERSION_MODULE_FILE_NAME_LENGTH, "forceAudioCodecName", forceAudioCodecName);
    ALOGD(OPTIONS_TAG, "%-*s: %s", VERSION_MODULE_FILE_NAME_LENGTH, "forceSubtitleCodecName", forceSubtitleCodecName);
    ALOGD(OPTIONS_TAG, "%-*s: %s", VERSION_MODULE_FILE_NAME_LENGTH, "forceVideoCodecName", forceVideoCodecName);
    ALOGD(OPTIONS_TAG, "%-*s: %lf", VERSION_MODULE_FILE_NAME_LENGTH, "rdftSpeed", rdftSpeed);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "infiniteBuffer", infiniteBuffer);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "showStatus", showStatus);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "findStreamInfo", findStreamInfo);
    showDict("player-opts", playerDict);
    showDict("format-opts", format);
    showDict("codec-opts ", codec);
    showDict("sws-opts   ", swsDict);
    showDict("swr-opts   ", swrDict);
    ALOGD(OPTIONS_TAG, "===== end =====");
}

void Options::showDict(const char *tag, AVDictionary *dict) {
    AVDictionaryEntry *t = nullptr;
    while ((t = av_dict_get(dict, "", t, AV_DICT_IGNORE_SUFFIX))) {
        ALOGD(OPTIONS_TAG, "%-*s: %-*s = %s", 12, tag, 28, t->key, t->value);
    }
}

Options::Options() {
}

Options::~Options() {
}

