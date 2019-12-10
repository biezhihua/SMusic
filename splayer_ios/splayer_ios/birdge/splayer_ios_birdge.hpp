#ifndef splayer_ios_birdge_h
#define splayer_ios_birdge_h

#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

extern bool IOS_DEBUG;

extern void (^__nonnull SwiftFunc)(void);

extern void CFuncTest(void);

typedef long long BirdgeContext;

void _create(BirdgeContext *context);

void _start(BirdgeContext *context);

void _stop(BirdgeContext *context);

void _pause(BirdgeContext *context);

void _play(BirdgeContext *context);

void _destroy(BirdgeContext *context);

void _reset(BirdgeContext *context);

void _seekTo(BirdgeContext *context, float timeMs);

void _setVolume(BirdgeContext *context, float leftVolume, float rightVolume);

void _setMute(BirdgeContext *context, bool mute);

void _setRate(BirdgeContext *context, float rate);

void _setPitch(BirdgeContext *context, float pitch);

void _setLooping(BirdgeContext *context, bool looping);

void _setSurface(BirdgeContext *context);

void _setDataSource(BirdgeContext *context, const char *path);

void _setDataSourceAndHeaders(BirdgeContext *context, const char *path, char *keys, void *values);

void _setOptionS(BirdgeContext *context, long category, const char *type, const char *option);

void _setOptionL(BirdgeContext *context, long category, const char *type, long option);

void _native_init(BirdgeContext *context);

long _getRotate(BirdgeContext *context);

long _getVideoWidth(BirdgeContext *context);

long _getVideoHeight(BirdgeContext *context);

bool _isPlaying(BirdgeContext *context);

long _getDuration(BirdgeContext *context);

long _getCurrentPosition(BirdgeContext *context);

bool _isLooping(BirdgeContext *context);

#ifdef __cplusplus
}
#endif

#endif
