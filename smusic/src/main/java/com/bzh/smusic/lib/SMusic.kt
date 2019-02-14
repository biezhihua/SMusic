package com.bzh.smusic.lib

import android.annotation.SuppressLint
import android.os.Looper
import android.util.Log
import androidx.annotation.Keep
import androidx.arch.core.executor.ArchTaskExecutor
import com.bzh.smusic.lib.annotations.CalledByNative

@SuppressLint("RestrictedApi")
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

    private fun isMainThread(): Boolean {
        return Looper.getMainLooper().thread === Thread.currentThread()
    }

    @CalledByNative
    @Keep
    fun onPlayerCreateFromNative() {
        ArchTaskExecutor.getMainThreadExecutor().execute {
            Log.d(TAG, "onPlayerCreateFromNative() called: isMainThread : " + isMainThread())
        }
    }

    @CalledByNative
    @Keep
    fun onPlayerStartFromNative() {
        ArchTaskExecutor.getMainThreadExecutor().execute {
            Log.d(TAG, "onPlayerStartFromNative() called: isMainThread : " + isMainThread())
            play()
        }
    }

    @CalledByNative
    @Keep
    fun onPlayerPlayFromNative() {
        ArchTaskExecutor.getMainThreadExecutor().execute {
            Log.d(TAG, "onPlayerPlayFromNative() called: isMainThread : " + isMainThread())
        }
    }

    @CalledByNative
    @Keep
    fun onPlayerPauseFromNative() {
        ArchTaskExecutor.getMainThreadExecutor().execute {
            Log.d(TAG, "onPlayerPauseFromNative() called: isMainThread : " + isMainThread())
        }
    }

    @CalledByNative
    @Keep
    fun onPlayerStopFromNative() {
        ArchTaskExecutor.getMainThreadExecutor().execute {
            Log.d(TAG, "onPlayerStopFromNative() called: isMainThread : " + isMainThread())
        }
    }

    @CalledByNative
    @Keep
    fun onPlayerDestroyFromNative() {
        ArchTaskExecutor.getMainThreadExecutor().execute {
            Log.d(TAG, "onPlayerDestroyFromNative() called: isMainThread : " + isMainThread())
        }
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