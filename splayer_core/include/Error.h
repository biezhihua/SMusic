#ifndef SPLAYER_ERROR_H
#define SPLAYER_ERROR_H

#include "ErrorNo.h"

#define S_MKTAG(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define S_MKBETAG(a, b, c, d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))

#define S_CORRECT   1
#define S_ERROR(e) (-(e)) /// Returns a negative error code from a POSIX error code, to return from library functions.

#define S_FFERRTAG(a, b, c, d) (-(int)S_MKTAG(a, b, c, d))

#define S_ERROR_BUG                S_FFERRTAG( 'B','U','G','!')
#define S_ERROR_EOF                S_FFERRTAG( 'E','O','F',' ')
#define S_ERROR_EXIT               S_FFERRTAG( 'E','X','I','T')
#define S_ERROR_UNKNOWN            S_FFERRTAG( 'U','N','K','N')

#endif //SPLAYER_ERROR_H
