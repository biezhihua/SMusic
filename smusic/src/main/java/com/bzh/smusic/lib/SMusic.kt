package com.bzh.smusic.lib

import android.util.Log
import androidx.annotation.Keep
import com.bzh.smusic.lib.annotations.CalledByNative

class SMusic {

    fun create() {
        Log.d(TAG, "create() called")
        nativeCreate()
    }

    fun setDataSource(source: String) {
        Log.d(TAG, "setDataSource() called with: pSource = [$source]")
        nativeSetSource(source)
    }


    fun start() {
        Log.d(TAG, "start() called")
        nativeStart()
    }

    fun play() {
        Log.d(TAG, "play() called")
        nativePlay()
    }

    fun pause() {
        Log.d(TAG, "pause() called")
        nativePause()
    }

    fun stop() {
        Log.d(TAG, "stop() called")
        nativeStop()
    }

    fun destroy() {
        Log.d(TAG, "destroy() called")
        nativeDestroy()
    }


    @CalledByNative
    @Keep
    fun onPlayerCreateFromNative() {
        Log.d(TAG, "onPlayerCreateFromNative() called")
    }

    @CalledByNative
    @Keep
    fun onPlayerStartFromNative() {
        Log.d(TAG, "onPlayerStartFromNative() called")
    }

    @CalledByNative
    @Keep
    fun onPlayerPlayFromNative() {
        Log.d(TAG, "onPlayerPlayFromNative() called")
    }

    @CalledByNative
    @Keep
    fun onPlayerPauseFromNative() {
        Log.d(TAG, "onPlayerPauseFromNative() called")
    }

    @CalledByNative
    @Keep
    fun onPlayerStopFromNative() {
        Log.d(TAG, "onPlayerStopFromNative() called")
    }

    @CalledByNative
    @Keep
    fun onPlayerDestroyFromNative() {
        Log.d(TAG, "onPlayerDestroyFromNative() called")
    }


    @Keep
    private external fun nativeSetSource(source: String)

    @Keep
    private external fun nativeCreate()

    @Keep
    private external fun nativeDestroy()

    @Keep
    private external fun nativeStart()

    @Keep
    private external fun nativePlay()

    @Keep
    private external fun nativePause()

    @Keep
    private external fun nativeStop()


    companion object {

        private const val TAG = "SMusic"

        init {
            System.loadLibrary("smusic")
            System.loadLibrary("sffmpeg")
        }
    }
}