#ifndef splayer_ios_birdge_h
#define splayer_ios_birdge_h

#include <stdbool.h>
#include <stdio.h>
#include "Log.h"
#ifdef __cplusplus
extern "C" {
#endif

extern  bool IOS_DEBUG;

extern void (^__nonnull SwiftFunc)(void);

extern void CFuncTest(void);

void _create(void *);

void _start();

void _stop();

void _pause();

void _play();

void _release();

void _reset();

void _seekTo(float timeMs);

void _setVolume(float leftVolume, float rightVolume);

void _setMute(bool mute);

void _setRate(float rate);

void _setPitch(float pitch);

void _setLooping(bool looping);

void _setSurface();

void _setDataSource(const char *path);

void _setDataSourceAndHeaders(const char *path, char *keys, void *values);

void _setOptionS(long category, const char *type, const char *option);

void _setOptionL(long category, const char *type, long option);

void _native_init();

long _getRotate();

long _getVideoWidth();

long _getVideoHeight();

bool _isPlaying();

long _getDuration();

long _getCurrentPosition();

bool _isLooping();

#ifdef __cplusplus
}
#endif

#endif
