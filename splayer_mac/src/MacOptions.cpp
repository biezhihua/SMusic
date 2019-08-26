#include "MacOptions.h"

MacOptions::MacOptions() = default;

void MacOptions::showOptions() {
    Options::showOptions();

    ALOGD(OPTIONS_TAG, "===== options =====");
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "borderLess", borderLess);
    ALOGD(OPTIONS_TAG, "%-*s: %d", VERSION_MODULE_FILE_NAME_LENGTH, "isFullScreen", isFullScreen);
    ALOGD(OPTIONS_TAG, "===== end =====");
    ALOGD(OPTIONS_TAG, "");
}
