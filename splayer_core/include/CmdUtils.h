#ifndef SPLAYER_CORE_CMDUTILS_H
#define SPLAYER_CORE_CMDUTILS_H

#include <stdbool.h>
#include <assert.h>
#include <libavutil/avstring.h>
#include <libavutil/time.h>
#include <libavformat/avformat.h>
#include <libavcodec/avfft.h>
#include <libswscale/swscale.h>
#include <libavutil/base64.h>
#include <libavutil/error.h>
#include <libavutil/opt.h>
#include <libavutil/version.h>
#include <libswresample/swresample.h>
#include <libavutil/display.h>
#include <libavutil/eval.h>
#include <libavutil/ffversion.h>
#include <config.h>

#define INDENT        1
#define SHOW_VERSION  2
#define SHOW_CONFIG   4
#define SHOW_COPYRIGHT 8

static int warned_cfg = 0;

#define PRINT_LIB_INFO(libname, LIBNAME, flags, level)                  \
    if (CONFIG_##LIBNAME) {                                             \
        const char *indent = flags & INDENT? "  " : "";                 \
        if (flags & SHOW_VERSION) {                                     \
            unsigned int version = libname##_version();                 \
            av_log(NULL, level,                                         \
                   "%slib%-11s %2d.%3d.%3d / %2d.%3d.%3d\n",            \
                   indent, #libname,                                    \
                   LIB##LIBNAME##_VERSION_MAJOR,                        \
                   LIB##LIBNAME##_VERSION_MINOR,                        \
                   LIB##LIBNAME##_VERSION_MICRO,                        \
                   AV_VERSION_MAJOR(version), AV_VERSION_MINOR(version),\
                   AV_VERSION_MICRO(version));                          \
        }                                                               \
        if (flags & SHOW_CONFIG) {                                      \
            const char *cfg = libname##_configuration();                \
            if (strcmp(FFMPEG_CONFIGURATION, cfg)) {                    \
                if (!warned_cfg) {                                      \
                    av_log(NULL, level,                                 \
                            "%sWARNING: library configuration mismatch\n", \
                            indent);                                    \
                    warned_cfg = 1;                                     \
                }                                                       \
                av_log(NULL, level, "%s%-11s configuration: %s\n",      \
                        indent, #libname, cfg);                         \
            }                                                           \
        }                                                               \
    }                                                                   \


void print_error(const char *filename, int err);

AVDictionary **setupFindStreamInfoOpts(AVFormatContext *s, AVDictionary *codec_opts);

AVDictionary *filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id,
                                AVFormatContext *s, AVStream *st, AVCodec *codec);

/**
 * Realloc array to hold new_size elements of elem_size.
 * Calls exit() on failure.
 *
 * @param array array to reallocate
 * @param elem_size size in bytes of each element
 * @param size new element count will be written here
 * @param new_size number of elements to place in reallocated array
 * @return reallocated array
 */
void *grow_array(void *array, int elem_size, int *size, int new_size);

#define GROW_ARRAY(array, nb_elems) array = grow_array(array, sizeof(*array), &nb_elems, nb_elems + 1)

double get_rotation(AVStream *st);

void show_ffmpeg_banner();

#endif //SPLAYER_CORE_CMDUTILS_H
