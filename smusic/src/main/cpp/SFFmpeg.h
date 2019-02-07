//
// Created by biezhihua on 2019/2/4.
//

#ifndef SMUSIC_S_FFMPEG_H
#define SMUSIC_S_FFMPEG_H

#include <string>

using namespace std;

class SFFmpeg {
public:

    string *source;
public:

    void setSource(string *pSource);

    void decodeFFmpeg();
};


#endif //SMUSIC_S_FFMPEG_H
