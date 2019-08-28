#ifndef SPLAYER_CORE_FRAME_H
#define SPLAYER_CORE_FRAME_H

extern "C" {
#include <libavutil/rational.h>
#include <libavutil/frame.h>
#include <libavformat/avformat.h>
};

/**
 * Common struct for handling all types of decoded data and allocated render buffers.
 */
class Frame {
public:
    AVFrame *frame;
    AVSubtitle subtitle;
    AVRational sampleAspectRatio;

    /// 序列，作seek时使用，作为区分前后帧序列
    int seekSerial;

    /// 帧的展示时间戳
    /// presentation timestamp for the frame
    double pts;

    /// 一帧的时长
    /// estimated duration of the frame
    double duration;

    /// 帧在文件中的字节位置
    /// byte position of the frame in the input file
    int64_t pos;

    /// 帧宽度
    int width;

    /// 帧高度
    int height;

    /// 帧格式
    int format;

    /// 是否已更新到Texture中
    int uploaded;

    /// 是否翻转图像
    int flipVertical;
};


#endif //SPLAYER_CORE_FRAME_H
