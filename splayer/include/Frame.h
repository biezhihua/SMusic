//
// Created by biezhihua on 2019-07-16.
//

#ifndef ANDROID_SPLAYER_FRAME_H
#define ANDROID_SPLAYER_FRAME_H

#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

/* Common struct for handling all types of decoded data and allocated render buffers. */
class Frame {
public:
    AVFrame *frame;
    AVSubtitle sub;
    int serial;
    double pts;           /* presentation timestamp for the frame */
    double duration;      /* estimated duration of the frame */
    int64_t pos;          /* byte position of the frame in the input file */
    int width;
    int height;
    int format;
    AVRational sar;
    int uploaded;
    int flip_v;
};


#endif //ANDROID_SPLAYER_FRAME_H
