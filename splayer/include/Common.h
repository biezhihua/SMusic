
#ifndef SPLAYER_COMMON_H
#define SPLAYER_COMMON_H

#ifndef S_FAILURE
#define S_FAILURE -1
#endif

#ifndef S_SUCCESS
#define S_SUCCESS 0
#endif


/* NOTE: the size must be big enough to compensate the hardware audio buffersize size */
/* TODO: We assume that a decoded and resampled frame fits into this buffer */
#define SAMPLE_ARRAY_SIZE (8 * 65536)

#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))



#define SHOW_MODE_NONE 0
#define SHOW_MODE_VIDEO 1
#define SHOW_MODE_WAVES 2
#define SHOW_MODE_NB 3


#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 25
#define EXTERNAL_CLOCK_MIN_FRAMES 2
#define EXTERNAL_CLOCK_MAX_FRAMES 10


#endif //SPLAYER_COMMON_H
