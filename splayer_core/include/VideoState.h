#ifndef SPLAYER_MAC_VIDEOSTATE_H
#define SPLAYER_MAC_VIDEOSTATE_H

extern "C" {
#include <libavformat/avformat.h>
};

class VideoState {

public:
    char *fileName;
    AVInputFormat *inputFormat;
    int yTop;
    int xLeft;
};

#endif //SPLAYER_MAC_VIDEOSTATE_H
