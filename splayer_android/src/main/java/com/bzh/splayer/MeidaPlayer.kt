@file:Suppress("FunctionName")

package com.bzh.splayer

import android.content.Context
import android.content.res.AssetFileDescriptor
import android.graphics.Rect
import android.net.Uri
import android.os.Handler
import android.os.Looper
import android.os.Message
import android.os.PowerManager
import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import androidx.annotation.NonNull
import com.bzh.splayer.annotations.AccessedByNative
import java.io.FileDescriptor
import java.io.IOException
import java.lang.ref.WeakReference

/**
 * 媒体播放器
 */
class MediaPlayer : IMediaPlayer {

    @AccessedByNative
    private val mNativeContext: Long = 0

    private var mSurfaceHolder: SurfaceHolder? = null

    private var mEventHandler: EventHandler? = null

    private var mWakeLock: PowerManager.WakeLock? = null

    private var mScreenOnWhilePlaying: Boolean = false

    private var mStayAwake: Boolean = false

    private var mOnListener: IMediaPlayer.OnListener? = null;

    override val rotate: Int
        get() = _getRotate()

    override val videoWidth: Int
        get() = _getVideoWidth()

    override val videoHeight: Int
        get() = _getVideoHeight()

    override val isPlaying: Boolean
        get() = _isPlaying()

    override val currentPosition: Long
        get() = _getCurrentPosition()

    override val duration: Long
        get() = _getDuration()

    override var isLooping: Boolean
        get() = _isLooping()
        set(looping) = _setLooping(looping)

    override var audioSessionId: Int = 0

    init {
        mEventHandler = when {
            Looper.myLooper() != null -> {
                EventHandler(this, Looper.myLooper()!!)
            }
            Looper.getMainLooper() != null -> {
                EventHandler(this, Looper.getMainLooper())
            }
            else -> null
        }
        _create(WeakReference(this))
    }

    override fun setDisplay(sh: SurfaceHolder?) {
        mSurfaceHolder = sh
        val surface = sh?.surface
        _setVideoSurface(surface)
        updateSurfaceScreenOn()
    }

    override fun setSurface(surface: Surface?) {
        if (mScreenOnWhilePlaying && surface != null) {
            Log.w(TAG, "setScreenOnWhilePlaying(true) is ineffective for Surface")
        }
        mSurfaceHolder = null
        _setVideoSurface(surface)
        updateSurfaceScreenOn()
    }

    override fun setDataSource(context: Context, uri: Uri) {
        setDataSource(context, uri, null)
    }

    override fun setDataSource(context: Context, uri: Uri, headers: Map<String, String>?) {
        val scheme = uri.scheme
        if (scheme == null || scheme == "file") {
            setDataSource(uri.path)
            return
        }

        var fd: AssetFileDescriptor? = null
        try {
            val resolver = context.contentResolver
            fd = resolver.openAssetFileDescriptor(uri, "r")
            if (fd == null) {
                return
            }
            // Note: using getDeclaredLength so that our behavior is the same
            // as previous versions when the content provider is returning
            // a full file.
            if (fd.declaredLength < 0) {
                setDataSource(fd.fileDescriptor)
            } else {
                setDataSource(fd.fileDescriptor, fd.startOffset, fd.declaredLength)
            }
            return
        } catch (ex: SecurityException) {
        } catch (ex: IOException) {
        } finally {
            fd?.close()
        }

        Log.d(TAG, "Couldn't open file on client side, trying server side")
        setDataSource(uri.toString(), headers)
    }

    override fun setDataSource(@NonNull path: String?) {
        _setDataSource(path)
    }

    override fun setDataSource(@NonNull path: String, headers: Map<String, String>?) {
        var keys: Array<String>? = null
        var values: Array<String>? = null

        if (headers != null) {
            val size = headers.size
            keys = Array(size) { it.toString() }
            values = Array(size) { it.toString() }

            var i = 0
            for ((key, value) in headers) {
                keys[i] = key
                values[i] = value
                ++i
            }
        }

        _setDataSource(path, keys, values)
    }

    override fun setDataSource(fd: FileDescriptor) {
        // intentionally less than LONG_MAX
        setDataSource(fd, 0, 0x7ffffffffffffffL)
    }

    override fun setDataSource(fd: FileDescriptor, offset: Long, length: Long) {
        _setDataSource(fd, offset, length)
    }

    override fun start() {
        if (DEBUG) {
            Log.d(TAG, "start() called")
        }
        stayAwake(true)
        _start()
    }

    override fun stop() {
        if (DEBUG) {
            Log.d(TAG, "stop() called")
        }
        stayAwake(false)
        _stop()
    }

    override fun pause() {
        if (DEBUG) {
            Log.d(TAG, "pause() called")
        }
        stayAwake(false)
        _pause()
    }

    override fun play() {
        if (DEBUG) {
            Log.d(TAG, "play() called")
        }
        stayAwake(true)
        _play()
    }

    override fun setWakeMode(context: Context, mode: Int) {
        var washeld = false
        if (mWakeLock != null) {
            if (mWakeLock!!.isHeld) {
                washeld = true
                mWakeLock!!.release()
            }
            mWakeLock = null
        }

        val pm = context.getSystemService(Context.POWER_SERVICE) as PowerManager
        mWakeLock =
            pm.newWakeLock(mode or PowerManager.ON_AFTER_RELEASE, MediaPlayer::class.java.name)
        mWakeLock?.setReferenceCounted(false)
        if (washeld) {
            mWakeLock?.acquire(0L)
        }
    }

    override fun setScreenOnWhilePlaying(screenOn: Boolean) {
        if (mScreenOnWhilePlaying != screenOn) {
            if (screenOn && mSurfaceHolder == null) {
                Log.w(TAG, "setScreenOnWhilePlaying(true) is ineffective without a SurfaceHolder")
            }
            mScreenOnWhilePlaying = screenOn
            updateSurfaceScreenOn()
        }
    }

    override fun seekTo(msec: Float) {
        _seekTo(msec)
    }

    override fun release() {
        stayAwake(false)
        updateSurfaceScreenOn()
        mOnListener = null
        _release()
    }

    override fun reset() {
        stayAwake(false)
        _reset()
        // make sure none of the listeners get called anymore
        mEventHandler!!.removeCallbacksAndMessages(null)
    }

    override fun setAudioStreamType(streamtype: Int) {
        // do nothing
    }

    fun setVolume(volume: Float) {
        setVolume(volume, volume)
    }

    override fun setVolume(leftVolume: Float, rightVolume: Float) {
        _setVolume(leftVolume, rightVolume)
    }

    override fun setMute(mute: Boolean) {
        _setMute(mute)
    }

    override fun setRate(rate: Float) {
        _setRate(rate)
    }

    override fun setPitch(pitch: Float) {
        _setPitch(pitch)
    }

    @Throws(IllegalStateException::class)
    fun setOption(category: Int, type: String, option: String) {
        _setOption(category, type, option)
    }

    @Throws(IllegalStateException::class)
    fun setOption(category: Int, type: String, option: Long) {
        _setOption(category, type, option)
    }

    @Throws(Throwable::class)
    protected fun finalize() {
        _native_finalize()
    }

    override fun setOnListener(listener: IMediaPlayer.OnListener) {
        mOnListener = listener
    }

    private fun stayAwake(awake: Boolean) {
        if (mWakeLock != null) {
            if (awake && !mWakeLock!!.isHeld) {
                mWakeLock?.acquire(0L)
            } else if (!awake && mWakeLock!!.isHeld) {
                mWakeLock?.release()
            }
        }
        mStayAwake = awake
        updateSurfaceScreenOn()
    }

    private fun updateSurfaceScreenOn() {
        if (mSurfaceHolder != null) {
            mSurfaceHolder!!.setKeepScreenOn(mScreenOnWhilePlaying && mStayAwake)
        }
    }

    private inner class EventHandler(private val mMediaPlayer: MediaPlayer, looper: Looper) :
        Handler(looper) {

        override fun handleMessage(msg: Message) {

            if (mMediaPlayer.mNativeContext == 0L) {
                Log.w(TAG, "mediaplayer went away with unhandled events")
                return
            }

            if (DEBUG) {
                Log.d(
                    TAG,
                    "handleMessage() called with: type = [${IMediaPlayer.MsgType.toString(msg.what)}] msg = [$msg]"
                )
            }

            when (msg.what) {

                IMediaPlayer.MsgType.MSG_PLAY_STARTED.value -> {
                    mOnListener?.onStarted(mMediaPlayer)
                    return
                }

                IMediaPlayer.MsgType.MSG_PLAY_COMPLETED.value -> {
                    mOnListener?.onCompletion(mMediaPlayer)
                    stayAwake(false)
                    return
                }

                IMediaPlayer.MsgType.MSG_BUFFERING_UPDATE.value -> {
                    mOnListener?.onBufferingUpdate(mMediaPlayer, msg.arg1)
                    return
                }

                IMediaPlayer.MsgType.MSG_SEEK_COMPLETE.value -> {
                    mOnListener?.onSeekComplete(mMediaPlayer)
                    return
                }

                IMediaPlayer.MsgType.MSG_VIDEO_SIZE_CHANGED.value -> {
                    mOnListener?.onVideoSizeChanged(
                        mMediaPlayer,
                        msg.arg1,
                        msg.arg2
                    )
                    return
                }

                IMediaPlayer.MsgType.MSG_ERROR.value -> {
                    // For PV specific error values (msg.arg2) look in
                    // opencore/pvmi/pvmf/include/pvmf_return_codes.h
                    Log.e(TAG, "Error (" + msg.arg1 + "," + msg.arg2 + ")")
                    var error_was_handled = false
                    if (mOnListener != null) {
                        error_was_handled =
                            mOnListener!!.onError(mMediaPlayer, msg.arg1, msg.arg2)
                    }
                    if (mOnListener != null && !error_was_handled) {
                        mOnListener!!.onCompletion(mMediaPlayer)
                    }
                    stayAwake(false)
                    return
                }

                IMediaPlayer.MsgType.MSG_BUFFERING_START.value, IMediaPlayer.MsgType.MSG_BUFFERING_END.value -> {
                    if (msg.arg1 != 0) {
                        Log.i(TAG, "Info (" + msg.arg1 + "," + msg.arg2 + ")")
                    }
                    mOnListener?.onInfo(mMediaPlayer, msg.arg1, msg.arg2)
                    // No real default action so far.
                    return
                }

                IMediaPlayer.MsgType.MSG_TIMED_TEXT.value -> {
                    if (mOnListener != null) {
                        if (msg.obj == null) {
                            mOnListener?.onTimedText(mMediaPlayer, null)
                        } else {
                            if (msg.obj is ByteArray) {
                                val text = TimedText(Rect(0, 0, 1, 1), msg.obj as String)
                                mOnListener?.onTimedText(mMediaPlayer, text)
                            }
                        }
                    }
                    return
                }

                IMediaPlayer.MsgType.MSG_CURRENT_POSITION.value -> {
                    mOnListener?.onCurrentPosition(
                        msg.arg1.toLong(),
                        msg.arg2.toLong()
                    )
                }

                else -> {
                    Log.e(TAG, "Unknown message $msg")
                    return
                }
            }
        }
    }

    @Throws(
        IOException::class, IllegalArgumentException::class, SecurityException::class,
        IllegalStateException::class
    )
    private external fun _setDataSource(@NonNull path: String?)

    @Throws(
        IOException::class, IllegalArgumentException::class, SecurityException::class,
        IllegalStateException::class
    )
    private external fun _setDataSource(
        path: String, keys: Array<String>?, values: Array<String>?
    )

    @Throws(IllegalStateException::class)
    private external fun _setOption(category: Int, type: String, option: Long)

    @Throws(IllegalStateException::class)
    private external fun _setOption(category: Int, type: String, option: String)

    @Throws(IllegalStateException::class)
    private external fun _create(mediaPlayerThis: Any)

    @Throws(IllegalStateException::class)
    private external fun _setLooping(looping: Boolean)

    @Throws(IllegalStateException::class)
    private external fun _isLooping(): Boolean

    @Throws(IllegalStateException::class)
    private external fun _setVolume(leftVolume: Float, rightVolume: Float)

    @Throws(IllegalStateException::class)
    private external fun _setMute(mute: Boolean)

    @Throws(IllegalStateException::class)
    private external fun _setRate(rate: Float)

    @Throws(IllegalStateException::class)
    private external fun _setPitch(pitch: Float)

    @Throws(IOException::class, IllegalArgumentException::class, IllegalStateException::class)
    private external fun _setDataSource(fd: FileDescriptor, offset: Long, length: Long)

    @Throws(IllegalStateException::class)
    private external fun _setVideoSurface(surface: Surface?)

    @Throws(IllegalStateException::class)
    private external fun _start()

    @Throws(IllegalStateException::class)
    private external fun _stop()

    @Throws(IllegalStateException::class)
    private external fun _pause()

    @Throws(IllegalStateException::class)
    private external fun _play()

    @Throws(IllegalStateException::class)
    private external fun _getRotate(): Int

    @Throws(IllegalStateException::class)
    private external fun _getVideoWidth(): Int

    @Throws(IllegalStateException::class)
    private external fun _getVideoHeight(): Int

    @Throws(IllegalStateException::class)
    private external fun _isPlaying(): Boolean

    @Throws(IllegalStateException::class)
    private external fun _seekTo(msec: Float)

    @Throws(IllegalStateException::class)
    private external fun _getCurrentPosition(): Long

    @Throws(IllegalStateException::class)
    private external fun _getDuration(): Long

    @Throws(IllegalStateException::class)
    private external fun _release()

    @Throws(IllegalStateException::class)
    private external fun _reset()

    @Throws(IllegalStateException::class)
    private external fun _native_finalize()

    companion object {

        @JvmStatic
        val DEBUG = true

        private const val TAG = "[MP][LIB][MediaPlayer]"

        // Options
        val OPT_CATEGORY_FORMAT = 1    // 解封装参数
        val OPT_CATEGORY_CODEC = 2     // 解码参数
        val OPT_CATEGORY_SWS = 3       // 视频转码参数
        val OPT_CATEGORY_PLAYER = 4    // 播放器参数
        val OPT_CATEGORY_SWR = 5       // 音频重采样参数

        init {
            System.loadLibrary("sffmpeg")
            System.loadLibrary("splayer_soundtouch")
            System.loadLibrary("splayer_renderer")
            System.loadLibrary("splayer_engine")
            System.loadLibrary("splayer_andorid")
            _native_init()
        }

        @JvmStatic
        fun create(context: Context, uri: Uri, holder: SurfaceHolder? = null): MediaPlayer? {

            try {
                val mp = MediaPlayer()
                mp.setDataSource(context, uri)
                if (holder != null) {
                    mp.setDisplay(holder)
                }
                mp.start()
                return mp
            } catch (ex: IOException) {
                Log.e(TAG, "init failed:", ex)
                // fall through
            } catch (ex: IllegalArgumentException) {
                Log.e(TAG, "init failed:", ex)
                // fall through
            } catch (ex: SecurityException) {
                Log.e(TAG, "init failed:", ex)
                // fall through
            }

            return null
        }

        @JvmStatic
        fun create(context: Context, resid: Int): MediaPlayer? {
            try {
                val afd = context.resources.openRawResourceFd(resid) ?: return null

                val mp = MediaPlayer()
                mp.setDataSource(afd.fileDescriptor, afd.startOffset, afd.length)
                afd.close()
                mp.start()
                return mp
            } catch (ex: IOException) {
                Log.e(TAG, "init failed:", ex)
                // fall through
            } catch (ex: IllegalArgumentException) {
                Log.e(TAG, "init failed:", ex)
                // fall through
            } catch (ex: SecurityException) {
                Log.e(TAG, "init failed:", ex)
                // fall through
            }

            return null
        }

        @JvmStatic
        private external fun _native_init()

        @JvmStatic
        fun postEventFromNative(
            mediaPlayerRef: Any,
            what: Int,
            arg1: Int,
            arg2: Int,
            obj: Any?
        ) {
            val ref = (mediaPlayerRef as WeakReference<*>).get() ?: return
            val mp = ref as MediaPlayer
            val m = mp.mEventHandler?.obtainMessage(what, arg1, arg2, obj)
            if (m != null) {
                mp.mEventHandler?.sendMessage(m)
            }
        }
    }
}
