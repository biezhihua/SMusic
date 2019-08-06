#ifndef SPLAYER_ERROR_H
#define SPLAYER_ERROR_H

#include "ErrorNo.h"

#define MKTAG(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define MKBETAG(a, b, c, d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))

#define POSITIVE   1
#define NEGATIVE(e) (-(e)) /// Returns a negative error code from a POSIX error code, to return from library functions.

#define FFERRTAG(a, b, c, d) (-(int)MKTAG(a, b, c, d))

#define NEGATIVE_BSF_NOT_FOUND      FFERRTAG(0xF8,'B','S','F') ///< Bitstream filter not found
#define NEGATIVE_BUG                FFERRTAG( 'B','U','G','!') ///< Internal bug, also see NEGATIVE_BUG2
#define NEGATIVE_BUFFER_TOO_SMALL   FFERRTAG( 'B','U','F','S') ///< Buffer too small
#define NEGATIVE_DECODER_NOT_FOUND  FFERRTAG(0xF8,'D','E','C') ///< Decoder not found
#define NEGATIVE_DEMUXER_NOT_FOUND  FFERRTAG(0xF8,'D','E','M') ///< Demuxer not found
#define NEGATIVE_ENCODER_NOT_FOUND  FFERRTAG(0xF8,'E','N','C') ///< Encoder not found
#define NEGATIVE_EOF                FFERRTAG( 'E','O','F',' ') ///< End of file
#define NEGATIVE_EXIT               FFERRTAG( 'E','X','I','T') ///< Immediate exit was requested; the called function should not be restarted
#define NEGATIVE_EXTERNAL           FFERRTAG( 'E','X','T',' ') ///< Generic error in an external library
#define NEGATIVE_FILTER_NOT_FOUND   FFERRTAG(0xF8,'F','I','L') ///< Filter not found
#define NEGATIVE_INVALIDDATA        FFERRTAG( 'I','N','D','A') ///< Invalid data found when processing input
#define NEGATIVE_MUXER_NOT_FOUND    FFERRTAG(0xF8,'M','U','X') ///< Muxer not found
#define NEGATIVE_OPTION_NOT_FOUND   FFERRTAG(0xF8,'O','P','T') ///< Option not found
#define NEGATIVE_PATCHWELCOME       FFERRTAG( 'P','A','W','E') ///< Not yet implemented in FFmpeg, patches welcome
#define NEGATIVE_PROTOCOL_NOT_FOUND FFERRTAG(0xF8,'P','R','O') ///< Protocol not found
#define NEGATIVE_UNKNOWN            FFERRTAG( 'U','N','K','N')

#endif //SPLAYER_ERROR_H
