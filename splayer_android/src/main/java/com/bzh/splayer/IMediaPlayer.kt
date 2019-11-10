@file:Suppress("KDocUnresolvedReference", "unused")

package com.bzh.splayer

import android.annotation.TargetApi
import android.content.Context
import android.net.Uri
import android.view.Surface
import android.view.SurfaceHolder
import androidx.annotation.NonNull
import java.io.FileDescriptor
import java.io.IOException

interface IMediaPlayer {

    val rotate: Int

    val videoWidth: Int

    val videoHeight: Int

    val isPlaying: Boolean

    val currentPosition: Long

    val duration: Long

    var isLooping: Boolean

    var audioSessionId: Int

    fun setDisplay(sh: SurfaceHolder?)

    fun setSurface(surface: Surface?)

    @Throws(
        IOException::class,
        IllegalArgumentException::class,
        SecurityException::class,
        IllegalStateException::class
    )
    fun setDataSource(@NonNull context: Context, @NonNull uri: Uri)

    @TargetApi(14)
    @Throws(
        IOException::class,
        IllegalArgumentException::class,
        SecurityException::class,
        IllegalStateException::class
    )
    fun setDataSource(@NonNull context: Context, @NonNull uri: Uri, headers: Map<String, String>?)

    @Throws(
        IOException::class,
        IllegalArgumentException::class,
        SecurityException::class,
        IllegalStateException::class
    )
    fun setDataSource(@NonNull path: String?)

    @Throws(
        IOException::class,
        IllegalArgumentException::class,
        SecurityException::class,
        IllegalStateException::class
    )
    fun setDataSource(@NonNull path: String, headers: Map<String, String>?)

    @Throws(IOException::class, IllegalArgumentException::class, IllegalStateException::class)
    fun setDataSource(fd: FileDescriptor)

    @Throws(IOException::class, IllegalArgumentException::class, IllegalStateException::class)
    fun setDataSource(fd: FileDescriptor, offset: Long, length: Long)

    @Throws(IllegalStateException::class)
    fun start()

    @Throws(IllegalStateException::class)
    fun stop()

    @Throws(IllegalStateException::class)
    fun pause()

    @Throws(IllegalStateException::class)
    fun play()

    fun setWakeMode(context: Context, mode: Int)

    fun setScreenOnWhilePlaying(screenOn: Boolean)

    @Throws(IllegalStateException::class)
    fun seekTo(msec: Float)

    fun release()

    fun reset()

    fun setAudioStreamType(streamtype: Int)

    fun setVolume(leftVolume: Float, rightVolume: Float)

    fun setMute(mute: Boolean)

    fun setRate(rate: Float)

    fun setPitch(pitch: Float)

    interface IOnListener {

        fun onStarted(mp: IMediaPlayer)

        fun onCompletion(mp: IMediaPlayer)

        fun onInfo(mp: IMediaPlayer, what: Int, extra: Int): Boolean

        fun onError(mp: IMediaPlayer, what: Int, extra: Int): Boolean

        fun onTimedText(mp: IMediaPlayer, text: ITimedText?)

        fun onVideoSizeChanged(mediaPlayer: IMediaPlayer, width: Int, height: Int)

        fun onSeekComplete(mp: IMediaPlayer)

        fun onBufferingUpdate(mp: IMediaPlayer, percent: Int)

        fun onCurrentPosition(current: Long, duration: Long)
    }

    fun setOnListener(listener: IOnListener)

    companion object {

    }

    enum class MsgType(var value: Int) {

        // 默认
        MSG_FLUSH(1000),

        // 出错
        MSG_ERROR(1001),

        // 改变状态
        MSG_CHANGE_STATUS(1002),

        // 播放开始
        MSG_PLAY_STARTED(1003),

        // 播放完成
        MSG_PLAY_COMPLETED(1004),

        // 打开文件
        MSG_OPEN_INPUT(1005),

        // 媒体流信息
        MSG_STREAM_INFO(1006),

        // 已准备解码器
        MSG_PREPARED_DECODER(1007),

        // 长宽比变化
        MSG_VIDEO_SIZE_CHANGED(1008),

        // 采样率变化
        MSG_SAR_CHANGED(1009),

        // 开始音频解码
        MSG_AUDIO_START(1010),

        // 音频渲染开始(播放开始)
        MSG_AUDIO_RENDERING_START(1011),

        // 视频渲染开始(渲染开始)
        MSG_VIDEO_START(1012),

        // 旋转角度变化
        MSG_VIDEO_ROTATION_CHANGED(1013),

        // 缓冲开始
        MSG_BUFFERING_START(1014),

        // 缓冲更新
        MSG_BUFFERING_UPDATE(1015),

        // 缓冲时间更新
        MSG_BUFFERING_TIME_UPDATE(1016),

        // 缓冲完成
        MSG_BUFFERING_END(1017),

        // 定位完成
        MSG_SEEK_START(1018),

        // 定位开始
        MSG_SEEK_COMPLETE(1019),

        // 字幕
        MSG_TIMED_TEXT(1020),

        // 当前时钟
        MSG_CURRENT_POSITION(1021);

        companion object {
            fun toString(value: Int): String {
                values().forEach {
                    if (it.value == value) {
                        return it.name
                    }
                }
                return "None"
            }
        }
    }

}