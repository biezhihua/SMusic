#ifndef SPLAYER_CORE_ERROR_H
#define SPLAYER_CORE_ERROR_H

/// Positive Value
#define POSITIVE                            1

/// Returns a Negative error code from a POSIX error code, to return from library functions.
#define NEGATIVE(e)                         (-(e))

#define IS_NEGATIVE(e)                      (e < 0)

#define S_ERROR                             1
#define S_NULL                              2
#define S_NOT_MEMORY                        6
#define S_NOT_OPEN_INPUT                    7
#define S_NOT_FIND_STREAM_INFO              8
#define S_NOT_OPEN_FILE                     9
#define S_NOT_ATTACHED_PIC                  10
#define S_ABORT_REQUEST                     11
#define S_NOT_PACKET                        12
#define S_NOT_VALID_STREAM_INDEX            13
#define S_NOT_CODEC_PARAMS_CONTEXT          14
#define S_NOT_OPEN_DECODE                   16
#define S_NOT_VIDEO_DECODE_START            18
#define S_NOT_SUBTITLE_DECODE_START         19
#define S_NOT_AUDIO_DECODE_START            20
#define S_NOT_DECODE_FRAME                  21
#define S_NOT_GET_PACKET_QUEUE              22
#define S_NOT_GET_VIDEO_FRAME               23
#define S_NOT_QUEUE_PICTURE                 24
#define S_NOT_FOUND_CODER                   25
#define S_NOT_FRAME_WRITEABLE               26
#define S_NOT_BLOCK_GET_MSG                 27
#define S_NOT_SDL_INIT                      28
#define S_NOT_SDL_CREATE_WINDOW             29
#define S_NOT_SDL_CREATE_RENDERER           30
#define S_NOT_REALLOC_TEXTURE               31
#define S_NOT_CREATE_TEXTURE                32
#define S_NOT_SET_BLEND_MODE                33
#define S_NOT_LOCK_TEXTURE                  34
#define S_NOT_UPDATE_YUV_TEXTURE            35
#define S_NOT_INIT_CONVERSION_CONTEXT       36
#define S_NOT_SUPPORT_LINESIZES             37
#define S_NOT_UPDATE_TEXTURE                38
#define S_NOT_CREATE_VIDEO_STATE            39
#define S_NOT_ALLOC_CODEC_CONTEXT           40
#define S_NOT_START_MSG_QUEUE               41
#define S_PREPARE_FAILURE                   42
#define S_NOT_CONFIGURE_AUDIO_FILTERS       43
#define S_NOT_CONFIGURE_VIDEO_FILTERS       44
#define S_NOT_OPEN_AUDIO                    44
#define S_SAMPLE_RATE_OR_CHANNEL_COUNT      45
#define S_NOT_SUPPORT_AUDIO_FORMAT          46
#define S_NOT_SUPPORT_AUDIO_CHANNEL_COUNT   47
#define S_NOT_SUPPORT_AUDIO_GET_BUFFER_SIZE 48
#define S_AUDIO_PAUSED                      49
#define S_NOT_FRAME_READABLE                50
#define S_NOT_CREATE_AUDIO_SWR_CONTEXT             51
#define S_AUDIO_OUT_SIZE                    52
#define S_NOT_AUDIO_SWR_COMPENSATION            53
#define S_NOT_CONVERT_AUDIO          54
#define S_NOT_DECODE_SUBTITLE_FRAME         55
#define S_FRAME_DROP                        56
#define S_NOT_ADD_FRAME_TO_FILTER           57
#define S_NOT_INIT_VIDEO_STATE              58
#define S_NOT_CREATE_STREAM                 59
#define S_DISABLE_ALL_STREAM                60
#define S_EOF                               61
#define S_NOT_FOUND_OPTION                  62

#endif //SPLAYER_CORE_ERROR_H
