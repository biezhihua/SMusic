package com.bzh.smusic.lib

import android.util.Log
import androidx.annotation.Keep
import com.bzh.smusic.lib.annotations.CalledByNative

class SMusic {

    private var source: String? = null

    private var listener: IMusicListener? = null

    fun prepare() {
        Log.d(TAG, "prepare() called")
        if (source == null) {
            return
        }
        nativePrepare(source!!)
    }

    fun start() {
        Log.d(TAG, "start() called")
        nativeStart()
    }

    fun setDataSource(source: String) {
        Log.d(TAG, "setDataSource() called with: pSource = [$source]")
        this.source = source
    }

    fun setListener(listener: IMusicListener) {
        this.listener = listener
    }

    @CalledByNative
    @Keep
    fun onPlayerCreateFromNative() {
        Log.d(TAG, "onPlayerCreateFromNative() called")
    }

    @CalledByNative
    @Keep
    fun onPlayerPrepareFromNative() {
        Log.d(TAG, "onPreparedCallFromNative() called")
        listener?.onPrepared()
    }

    @CalledByNative
    @Keep
    fun onPlayerPlayFromNative() {
        Log.d(TAG, "onPlayerPlayFromNative() called")
    }

    @CalledByNative
    @Keep
    fun onPlayerStopFromNative() {
        Log.d(TAG, "onPlayerPlayFromNative() called")
    }

    @CalledByNative
    @Keep
    fun onPlayerDestroyFromNative() {
        Log.d(TAG, "onPlayerPlayFromNative() called")
    }


    @Keep
    external fun nativeInit()

    @Keep
    external fun nativeDestroy()

    @Keep
    private external fun nativePrepare(source: String)

    @Keep
    private external fun nativeStart()

    companion object {

        private const val TAG = "SMusic"

        init {
            System.loadLibrary("smusic")
            System.loadLibrary("sffmpeg")
        }
    }
}