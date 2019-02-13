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
        Log.d(TAG, "onPlayerPrepareFromNative() called")
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
        Log.d(TAG, "onPlayerStopFromNative() called")
    }

    @CalledByNative
    @Keep
    fun onPlayerDestroyFromNative() {
        Log.d(TAG, "onPlayerDestroyFromNative() called")
    }

    fun init() {
        Log.d(TAG, "init() called")
        nativeInit()
    }


    fun release() {
        Log.d(TAG, "release() called")
        nativeRelease()
    }

    @Keep
    private external fun nativeInit()

    @Keep
    private external fun nativeRelease()

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