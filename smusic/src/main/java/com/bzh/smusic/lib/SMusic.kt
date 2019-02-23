package com.bzh.smusic.lib

import android.annotation.SuppressLint
import android.os.Looper
import android.util.Log
import androidx.annotation.Keep
import androidx.annotation.WorkerThread
import androidx.arch.core.executor.ArchTaskExecutor
import com.bzh.smusic.lib.annotations.CalledByNative

@SuppressLint("RestrictedApi")
class SMusic {

    enum class Mute(val value: Int) {
        LEFT(0),
        RIGHT(1),
        CENTER(2)


    }

    var listener: IMusicListener? = null

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

    fun seek(progress: Int) {
        Log.d(TAG, "seek() called with: progress = [$progress]")
        nativeSeek(progress)
    }

    fun volume(percent: Int) {
        Log.d(TAG, "volume() called with: percent = [$percent]")
        nativeVolume(percent)
    }

    fun getCurrentVolume(): Int {
        val volume = nativeGetCurrentVolume()
        Log.d(TAG, "getCurrentVolume() called: $volume")
        return volume
    }

    fun mute(mute: Mute) {
        nativeMute(mute.value)
    }

    private fun isMainThread(): Boolean {
        return Looper.getMainLooper().thread === Thread.currentThread()
    }

    @WorkerThread
    @CalledByNative
    @Keep
    fun onPlayerCreateFromNative() {
        ArchTaskExecutor.getMainThreadExecutor().execute {
            Log.d(TAG, "onPlayerCreateFromNative() called: isMainThread : " + isMainThread())
            listener?.onCreate()
        }
    }

    @WorkerThread
    @CalledByNative
    @Keep
    fun onPlayerStartFromNative() {
        ArchTaskExecutor.getMainThreadExecutor().execute {
            Log.d(TAG, "onPlayerStartFromNative() called: isMainThread : " + isMainThread())
            listener?.onStart()
            play()
        }
    }

    @WorkerThread
    @CalledByNative
    @Keep
    fun onPlayerPlayFromNative() {
        ArchTaskExecutor.getMainThreadExecutor().execute {
            Log.d(TAG, "onPlayerPlayFromNative() called: isMainThread : " + isMainThread())
            listener?.onPlay()
        }
    }

    @WorkerThread
    @CalledByNative
    @Keep
    fun onPlayerPauseFromNative() {
        ArchTaskExecutor.getMainThreadExecutor().execute {
            Log.d(TAG, "onPlayerPauseFromNative() called: isMainThread : " + isMainThread())
            listener?.onPause()
        }
    }

    @WorkerThread
    @CalledByNative
    @Keep
    fun onPlayerStopFromNative() {
        ArchTaskExecutor.getMainThreadExecutor().execute {
            Log.d(TAG, "onPlayerStopFromNative() called: isMainThread : " + isMainThread())
            listener?.onStop()
        }
    }

    @WorkerThread
    @CalledByNative
    @Keep
    fun onPlayerDestroyFromNative() {
        ArchTaskExecutor.getMainThreadExecutor().execute {
            Log.d(TAG, "onPlayerDestroyFromNative() called: isMainThread : " + isMainThread())
            listener?.onDestroy()
        }
    }

    @WorkerThread
    @CalledByNative
    @Keep
    fun onPlayerTimeFromNative(totalTime: Int, currentTime: Int) {
        ArchTaskExecutor.getMainThreadExecutor().execute {
            Log.d(TAG, "onPlayerTimeFromNative() called $totalTime $currentTime")
            listener?.onTime(totalTime, currentTime)
        }
    }

    @WorkerThread
    @CalledByNative
    @Keep
    fun onPlayerErrorFromNative(code: Int, message: String) {
        ArchTaskExecutor.getMainThreadExecutor().execute {
            Log.d(TAG, "onPlayerError() called $code $message")
            listener?.onError(code, message)
        }
    }

    @WorkerThread
    @CalledByNative
    @Keep
    fun onPlayerCompleteFromNative() {
        ArchTaskExecutor.getMainThreadExecutor().execute {
            Log.d(TAG, "onPlayerCompleteFromNative() called: isMainThread : " + isMainThread())
            listener?.onComplete()
        }
    }


    @WorkerThread
    @CalledByNative
    @Keep
    fun onPlayerLoadStateFromNative(loadState: Boolean) {
        ArchTaskExecutor.getMainThreadExecutor().execute {
            Log.d(TAG, "onPlayerLoadingFromNative() called $loadState")
            listener?.onLoadState(loadState)
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

    @Keep
    private external fun nativeSeek(seek: Int)

    @Keep
    private external fun nativeVolume(percent: Int)

    @Keep
    private external fun nativeMute(mute: Int)

    @Keep
    private external fun nativeGetTotalTimeMillis(): Int

    @Keep
    private external fun nativeGetCurrentTimeMillis(): Int

    @Keep
    private external fun nativeGetCurrentVolume(): Int

    companion object {

        private const val TAG = "SMusic"

        init {
            System.loadLibrary("smusic")
            System.loadLibrary("sffmpeg")
        }
    }
}