#ifndef SPLAYER_MAC_DEFINE_H
#define SPLAYER_MAC_DEFINE_H

#define VERSION_MODULE_FILE_NAME_LENGTH     13

// Queue Size
#define VIDEO_QUEUE_SIZE        3
#define AUDIO_QUEUE_SIZE        9
#define SUBTITLE_QUEUE_SIZE     16
#define FRAME_QUEUE_SIZE        FFMAX(AUDIO_QUEUE_SIZE, FFMAX(VIDEO_QUEUE_SIZE, SUBTITLE_QUEUE_SIZE))
#define MAX_QUEUE_SIZE (15 * 1024 * 1024)

#define MIN_FRAMES 25

#define EXTERNAL_CLOCK_MIN_FRAMES 2
#define EXTERNAL_CLOCK_MAX_FRAMES 10

// Volume
#define MIX_MAX_VOLUME      128

// Sync Type
#define SYNC_TYPE_AUDIO_MASTER        0 /* default choice */
#define SYNC_TYPE_VIDEO_MASTER        1
#define SYNC_TYPE_EXTERNAL_CLOCK      2 /* synchronize to an external clock */

// Show Mode
#define SHOW_MODE_NONE      -1
#define SHOW_MODE_VIDEO     0
#define SHOW_MODE_WAVES     1
#define SHOW_MODE_RDFT      2
#define SHOW_MODE_NB        3

/* no AV sync correction is done if below the minimum AV sync threshold */
#define SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define SYNC_FRAMEDUP_THRESHOLD 0.1
/* no AV correction is done if too big error */
#define NOSYNC_THRESHOLD 10.0

/* polls for possible required screen refresh at least this often, should be less than 1/fps */
#define REFRESH_RATE 0.01

/* external clock speed adjustment constants for realtime sources based on buffer fullness */
#define EXTERNAL_CLOCK_SPEED_MIN  0.900
#define EXTERNAL_CLOCK_SPEED_MAX  1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001

// 1 second
#define CURSOR_HIDE_DELAY (1*1000000)

#endif //SPLAYER_MAC_DEFINE_H
