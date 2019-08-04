#ifndef SPLAYER_MAC_DEFINE_H
#define SPLAYER_MAC_DEFINE_H

#define VERSION_MODULE_FILE_NAME_LENGTH     13

// Queue Size
#define VIDEO_QUEUE_SIZE        3
#define AUDIO_QUEUE_SIZE        9
#define SUBTITLE_QUEUE_SIZE     16
#define FRAME_QUEUE_SIZE        FFMAX(AUDIO_QUEUE_SIZE, FFMAX(VIDEO_QUEUE_SIZE, SUBTITLE_QUEUE_SIZE))

#define MIX_MAX_VOLUME      128

// Sync Type
#define AV_SYNC_AUDIO_MASTER        0 /* default choice */
#define AV_SYNC_VIDEO_MASTER        1
#define AV_SYNC_EXTERNAL_CLOCK      2 /* synchronize to an external clock */

// Show Mode
#define SHOW_MODE_NONE      -1
#define SHOW_MODE_VIDEO     0
#define SHOW_MODE_WAVES     1
#define SHOW_MODE_RDFT      2
#define SHOW_MODE_NB        3

#endif //SPLAYER_MAC_DEFINE_H
