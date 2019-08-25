#ifndef SPLAYER_MAC_MACOPTIONS_H
#define SPLAYER_MAC_MACOPTIONS_H

#include "Options.h"

class MacOptions : public Options {

public:

    /// 窗口无边框
    int borderLess = 0; // borderLess window

    /// 光标是否隐藏
    int cursorHidden = 0;

    /// 光标最后显示时间
    int64_t cursorLastShown = 0;

    /// 是否全屏
    int isFullScreen = 0;

    MacOptions();

    void showOptions() override;
};


#endif //SPLAYER_MAC_MACOPTIONS_H
