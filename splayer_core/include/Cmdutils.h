
#ifndef SPLAYER_MAC_CMDUTILS_H
#define SPLAYER_MAC_CMDUTILS_H

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

void print_error(const char *filename, int err);

AVDictionary **setup_find_stream_info_opts(AVFormatContext *s, AVDictionary *codec_opts);

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

#endif //SPLAYER_MAC_CMDUTILS_H
