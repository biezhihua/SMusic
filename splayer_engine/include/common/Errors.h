#ifndef ERRORS_H
#define ERRORS_H

#include <sys/types.h>
#include <cerrno>

#define  SUCCESS                            1
#define  ERROR                              -1

#define  ERROR_PARAMS                       -2
#define  ERROR_NOT_MEMORY                   -3
#define  ERROR_NOT_OPEN_INPUT               -4
#define  ERROR_CODEC_OPTIONS                -5
#define  ERROR_NOT_FOUND_STREAM_INFO        -6
#define  ERROR_DISABLE_ALL_STREAM           -7
#define  ERROR_CREATE_VIDEO_AUDIO_DECODER   -8
#define  ERROR_NOT_FOUND_DCODE              -9
#define  ERROR_NOT_OPEN_DECODE              -10
#define  ERROR_EOF                          -11
#define  ERROR_IO                           -12
#define  ERROR_STREAM_INDEX                 -13
#define  ERROR_COPY_CODEC_PARAM_TO_CONTEXT  -14
#define  ERROR_CREATE_VIDEO_DECODER         -15
#define  ERROR_CREATE_AUDIO_DECODER         -16
#define  ERROR_ABORT_REQUEST                -17
#define  ERROR_FRAME_QUEUE_NOT_WRITABLE     -19
#define  ERROR_AUDIO_SPEC                   -20
#define  ERROR_AUDIO_FORMAT                 -21
#define  ERROR_AUDIO_CHANNEL_LAYOUT         -22
#define  ERROR_AUDIO_DECODE                 -23
#define  ERROR_AUDIO_SWR                    -24
#define  ERROR_AUDIO_OUT_SIZE               -25
#define  ERROR_AUDIO_SWR_COMPENSATION       -26
#define  ERROR_AUDIO_SWR_CONVERT            -27
#define  ERROR_AUDIO_OPEN                   -28


#endif //ERRORS_H
