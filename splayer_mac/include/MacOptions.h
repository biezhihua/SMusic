#ifndef SPLAYER_MAC_MACOPTIONS_H
#define SPLAYER_MAC_MACOPTIONS_H

#include "Options.h"

class MacOptions : public Options {

public:
    int borderLess = 0; // borderLess window
    int cursorHidden = 0;
    int64_t cursorLastShown = 0;
};


#endif //SPLAYER_MAC_MACOPTIONS_H
