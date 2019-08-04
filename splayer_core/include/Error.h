#ifndef SPLAYER_ERROR_H
#define SPLAYER_ERROR_H

#include "ErrorNo.h"

#define S_MKTAG(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define S_MKBETAG(a, b, c, d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))

#define S_CORRECT   1
#define S_ERROR(e) (-(e)) /// Returns a negative error code from a POSIX error code, to return from library functions.

#define S_FFERRTAG(a, b, c, d) (-(int)S_MKTAG(a, b, c, d))

#define SE_BSF_NOT_FOUND      S_FFERRTAG(0xF8,'B','S','F') ///< Bitstream filter not found
#define SE_BUG                S_FFERRTAG( 'B','U','G','!') ///< Internal bug, also see SE_BUG2
#define SE_BUFFER_TOO_SMALL   S_FFERRTAG( 'B','U','F','S') ///< Buffer too small
#define SE_DECODER_NOT_FOUND  S_FFERRTAG(0xF8,'D','E','C') ///< Decoder not found
#define SE_DEMUXER_NOT_FOUND  S_FFERRTAG(0xF8,'D','E','M') ///< Demuxer not found
#define SE_ENCODER_NOT_FOUND  S_FFERRTAG(0xF8,'E','N','C') ///< Encoder not found
#define SE_EOF                S_FFERRTAG( 'E','O','F',' ') ///< End of file
#define SE_EXIT               S_FFERRTAG( 'E','X','I','T') ///< Immediate exit was requested; the called function should not be restarted
#define SE_EXTERNAL           S_FFERRTAG( 'E','X','T',' ') ///< Generic error in an external library
#define SE_FILTER_NOT_FOUND   S_FFERRTAG(0xF8,'F','I','L') ///< Filter not found
#define SE_INVALIDDATA        S_FFERRTAG( 'I','N','D','A') ///< Invalid data found when processing input
#define SE_MUXER_NOT_FOUND    S_FFERRTAG(0xF8,'M','U','X') ///< Muxer not found
#define SE_OPTION_NOT_FOUND   S_FFERRTAG(0xF8,'O','P','T') ///< Option not found
#define SE_PATCHWELCOME       S_FFERRTAG( 'P','A','W','E') ///< Not yet implemented in FFmpeg, patches welcome
#define SE_PROTOCOL_NOT_FOUND S_FFERRTAG(0xF8,'P','R','O') ///< Protocol not found
#define SE_UNKNOWN            S_FFERRTAG( 'U','N','K','N')

#endif //SPLAYER_ERROR_H
