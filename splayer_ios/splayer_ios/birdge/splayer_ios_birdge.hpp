#ifndef splayer_ios_birdge_h
#define splayer_ios_birdge_h

#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

extern bool IOS_DEBUG;

typedef long long _NMPReference;

typedef void _SMPReference;

typedef void (*_PostFromNativeReference)(_SMPReference *_Nullable smpReference, int msg, int ext1, int ext2);

extern void (*_postFromNative)(_SMPReference *_Nullable smpReference, int msg, int ext1, int ext2);

void _create(_NMPReference *_Nullable nmpReference, _SMPReference *_Nullable smpReference);

void _start(_NMPReference *_Nullable nmpReference);

void _stop(_NMPReference *_Nullable nmpReference);

void _pause(_NMPReference *_Nullable nmpReference);

void _play(_NMPReference *_Nullable nmpReference);

void _destroy(_NMPReference *_Nullable nmpReference);

void _reset(_NMPReference *_Nullable nmpReference);

void _seekTo(_NMPReference *_Nullable nmpReference, float timeMs);

void _setVolume(_NMPReference *_Nullable nmpReference, float leftVolume, float rightVolume);

void _setMute(_NMPReference *_Nullable nmpReference, bool mute);

void _setRate(_NMPReference *_Nullable nmpReference, float rate);

void _setPitch(_NMPReference *_Nullable nmpReference, float pitch);

void _setLooping(_NMPReference *_Nullable nmpReference, bool looping);

void _setSurface(_NMPReference *_Nullable nmpReference);

void _setDataSource(_NMPReference *_Nullable nmpReference, const char *_Nullable path);

void _setDataSourceAndHeaders(_NMPReference *_Nullable nmpReference, const char *_Nullable path, char *_Nullable keys, void *_Nullable values);

void _setOptionS(_NMPReference *_Nullable nmpReference, long category, const char *_Nullable type, const char *_Nullable option);

void _setOptionL(_NMPReference *_Nullable nmpReference, long category, const char *_Nullable type, long option);

void _native_init(_NMPReference *_Nullable nmpReference);

long _getRotate(_NMPReference *_Nullable nmpReference);

long _getVideoWidth(_NMPReference *_Nullable nmpReference);

long _getVideoHeight(_NMPReference *_Nullable nmpReference);

bool _isPlaying(_NMPReference *_Nullable nmpReference);

long _getDuration(_NMPReference *_Nullable nmpReference);

long _getCurrentPosition(_NMPReference *_Nullable nmpReference);

bool _isLooping(_NMPReference *_Nullable nmpReference);

#ifdef __cplusplus
}
#endif

#endif
