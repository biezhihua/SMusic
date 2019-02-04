package com.bzh.smusic

import android.util.Log
import androidx.annotation.Keep
import com.bzh.smusic.annotations.CalledByNative

class SMusic {

    private var source: String? = null

    private var listener: IMusicListener? = null

    fun prepare() {
        Log.d(TAG, "prepare() called")
        if (source == null) {
            return
        }
        Thread {
            kotlin.run {
                nativePrepare(source!!)
            }
        }.start()
    }

    fun start() {
        Log.d(TAG, "start() called")
        Thread {
            kotlin.run {
                nativeStart()
            }
        }.start()
    }

    fun setDataSource(source: String) {
        Log.d(TAG, "setDataSource() called with: source = [$source]")
        this.source = source
    }

    fun setListener(listener: IMusicListener) {
        this.listener = listener
    }

    @CalledByNative
    @Keep
    fun onPreparedCallFromNative() {
        Log.d(TAG, "onPreparedCallFromNative() called")
        listener?.onPrepared()
    }

    @Keep
    external fun nativePrepare(source: String)

    @Keep
    external fun nativeStart()

    companion object {

        private const val TAG = "SMusic"

        init {
            System.loadLibrary("smusic")
            System.loadLibrary("sffmpeg")
        }
    }
}