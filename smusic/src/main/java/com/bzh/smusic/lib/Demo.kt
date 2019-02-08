package com.bzh.smusic.lib

import android.util.Log
import androidx.annotation.Keep
import androidx.annotation.MainThread
import androidx.annotation.WorkerThread

class Demo {

    external fun stringFromJNI(): String

    external fun testFFmpeg()

    external fun normalThread()

    external fun mutexThread()

    external fun mainThreadCallJava()

    external fun subThreadCallJava()

    external fun mainThreadCallStaticJava()

    external fun subThreadCallStaticJava()

    @MainThread
    @Keep
    fun onMainThreadError(code: Int, msg: String) {
        Log.d(TAG, "onMainThreadError() called with: code = [$code], msg = [$msg]")
    }

    @WorkerThread
    @Keep
    fun onSubThreadError(code: Int, msg: String) {
        Log.d(TAG, "onSubThreadError() called with: code = [$code], msg = [$msg]")
    }

    companion object {

        private const val TAG = "Demo"

        @MainThread
        @Keep
        @JvmStatic
        fun onMainThreadStaticError(code: Int, msg: String) {
            Log.d(TAG, "onMainThreadStaticError() called with: code = [$code], msg = [$msg]")
        }

        @WorkerThread
        @Keep
        @JvmStatic
        fun onSubThreadStaticError(code: Int, msg: String) {
            Log.d(TAG, "onSubThreadStaticError() called with: code = [$code], msg = [$msg]")
        }

        init {
            System.loadLibrary("smusic")
            System.loadLibrary("sffmpeg")
        }
    }
}