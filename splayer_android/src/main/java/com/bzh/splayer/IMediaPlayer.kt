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

    /**
     * Returns the rotate of the video.
     *
     * @return the rotate of the video, or o if there is no video
     */
    val rotate: Int

    /**
     * Returns the width of the video.
     *
     * @return the width of the video, or 0 if there is no video,
     * no display surface was set, or the width has not been determined
     * yet. The OnVideoSizeChangedListener can be registered via
     * [.setOnVideoSizeChangedListener]
     * to provide a notification when the width is available.
     */
    val videoWidth: Int

    /**
     * Returns the height of the video.
     *
     * @return the height of the video, or 0 if there is no video,
     * no display surface was set, or the height has not been determined
     * yet. The OnVideoSizeChangedListener can be registered via
     * [.setOnVideoSizeChangedListener]
     * to provide a notification when the height is available.
     */
    val videoHeight: Int

    /**
     * Checks whether the MediaPlayer is playing.
     *
     * @return true if currently playing, false otherwise
     */
    val isPlaying: Boolean

    /**
     * Gets the current playback position.
     *
     * @return the current position in milliseconds
     */
    val currentPosition: Long

    /**
     * Gets the duration of the file.
     *
     * @return the duration in milliseconds
     */
    val duration: Long

    /**
     * Checks whether the MediaPlayer is looping or non-looping.
     *
     * @return true if the MediaPlayer is currently looping, false otherwise
     */
    /**
     * Sets the player to be looping or non-looping.
     *
     * @param looping whether to loop or not
     */
    var isLooping: Boolean

    /**
     * Returns the audio session ID.
     *
     * @return the audio session ID. {@see #setAudioSessionId(int)}
     * Note that the audio session ID is 0 only if a problem occured when the MediaPlayer was contructed.
     */
    /**
     * Sets the audio session ID.
     *
     * @param sessionId the audio session ID.
     * The audio session ID is a system wide unique identifier for the audio stream played by
     * this MediaPlayer instance.
     * The primary use of the audio session ID  is to associate audio effects to a particular
     * instance of MediaPlayer: if an audio session ID is provided when creating an audio effect,
     * this effect will be applied only to the audio content of media players within the same
     * audio session and not to the output mix.
     * When created, a MediaPlayer instance automatically generates its own audio session ID.
     * However, it is possible to force this player to be part of an already existing audio session
     * by calling this method.
     * This method must be called before one of the overloaded ` setDataSource ` methods.
     * @throws IllegalStateException if it is called in an invalid state
     */
    var audioSessionId: Int

    /**
     * Sets the [SurfaceHolder] to use for displaying the video
     * portion of the media.
     *
     * Either a surface holder or surface must be set if a display or video sink
     * is needed.  Not calling this method or [.setSurface]
     * when playing back a video will result in only the audio track being played.
     * A null surface holder or surface will result in only the audio track being
     * played.
     *
     * @param sh the SurfaceHolder to use for video display
     */
    fun setDisplay(sh: SurfaceHolder?)

    /**
     * Sets the [Surface] to be used as the sink for the video portion of
     * the media. This is similar to [.setDisplay], but
     * does not support [.setScreenOnWhilePlaying].  Setting a
     * Surface will un-set any Surface or SurfaceHolder that was previously set.
     * A null surface will result in only the audio track being played.
     *
     * If the Surface sends frames to a [SurfaceTexture], the timestamps
     * returned from [SurfaceTexture.getTimestamp] will have an
     * unspecified zero point.  These timestamps cannot be directly compared
     * between different media sources, different instances of the same media
     * source, or multiple runs of the same program.  The timestamp is normally
     * monotonically increasing and is unaffected by time-of-day adjustments,
     * but it is reset when the position is set.
     *
     * @param surface The [Surface] to be used for the video portion of
     * the media.
     */
    fun setSurface(surface: Surface?)

    /**
     * Sets the data source as a content Uri.
     *
     * @param context the Context to use when resolving the Uri
     * @param uri the Content URI of the data you want to play
     * @throws IllegalStateException if it is called in an invalid state
     */
    @Throws(
        IOException::class,
        IllegalArgumentException::class,
        SecurityException::class,
        IllegalStateException::class
    )
    fun setDataSource(@NonNull context: Context, @NonNull uri: Uri)

    /**
     * Sets the data source as a content Uri.
     *
     * @param context the Context to use when resolving the Uri
     * @param uri the Content URI of the data you want to play
     * @param headers the headers to be sent together with the request for the data
     * @throws IllegalStateException if it is called in an invalid state
     */
    @TargetApi(14)
    @Throws(
        IOException::class,
        IllegalArgumentException::class,
        SecurityException::class,
        IllegalStateException::class
    )
    fun setDataSource(@NonNull context: Context, @NonNull uri: Uri, headers: Map<String, String>?)


    /**
     * Sets the data source (file-path or http/rtsp URL) to use.
     *
     * @param path the path of the file, or the http/rtsp URL of the stream you want to play
     * @throws IllegalStateException if it is called in an invalid state
     */
    @Throws(
        IOException::class,
        IllegalArgumentException::class,
        SecurityException::class,
        IllegalStateException::class
    )
    fun setDataSource(@NonNull path: String?)

    /**
     * Sets the data source (file-path or http/rtsp URL) to use.
     *
     * @param path the path of the file, or the http/rtsp URL of the stream you want to play
     * @param headers the headers associated with the http request for the stream you want to play
     * @throws IllegalStateException if it is called in an invalid state
     * @hide pending API council
     */
    @Throws(
        IOException::class,
        IllegalArgumentException::class,
        SecurityException::class,
        IllegalStateException::class
    )
    fun setDataSource(@NonNull path: String, headers: Map<String, String>?)

    /**
     * Sets the data source (FileDescriptor) to use. It is the caller's responsibility
     * to close the file descriptor. It is safe to do so as soon as this call returns.
     *
     * @param fd the FileDescriptor for the file you want to play
     * @throws IllegalStateException if it is called in an invalid state
     */
    @Throws(IOException::class, IllegalArgumentException::class, IllegalStateException::class)
    fun setDataSource(fd: FileDescriptor)

    /**
     * Sets the data source (FileDescriptor) to use.  The FileDescriptor must be
     * seekable (N.B. a LocalSocket is not seekable). It is the caller's responsibility
     * to close the file descriptor. It is safe to do so as soon as this call returns.
     *
     * @param fd the FileDescriptor for the file you want to play
     * @param offset the offset into the file where the data to be played starts, in bytes
     * @param length the length in bytes of the data to be played
     * @throws IllegalStateException if it is called in an invalid state
     */
    @Throws(IOException::class, IllegalArgumentException::class, IllegalStateException::class)
    fun setDataSource(fd: FileDescriptor, offset: Long, length: Long)

    /**
     * Prepares the player for playback, synchronously.
     *
     * After setting the datasource and the display surface, you need to either
     * call prepare() or prepareAsync(). For files, it is OK to call prepare(),
     * which blocks until MediaPlayer is ready for playback.
     *
     * @throws IllegalStateException if it is called in an invalid state
     */
    @Throws(IOException::class, IllegalStateException::class)
    fun prepare()

    /**
     * Prepares the player for playback, asynchronously.
     *
     * After setting the datasource and the display surface, you need to either
     * call prepare() or prepareAsync(). For streams, you should call prepareAsync(),
     * which returns immediately, rather than blocking until enough data has been
     * buffered.
     *
     * @throws IllegalStateException if it is called in an invalid state
     */
    @Throws(IllegalStateException::class)
    fun prepareAsync()

    /**
     * Starts or resumes playback. If playback had previously been paused,
     * playback will continue from where it was paused. If playback had
     * been stopped, or never started before, playback will start at the
     * beginning.
     *
     * @throws IllegalStateException if it is called in an invalid state
     */
    @Throws(IllegalStateException::class)
    fun start()

    /**
     * Stops playback after playback has been stopped or paused.
     *
     * @throws IllegalStateException if the internal player engine has not been
     * initialized.
     */
    @Throws(IllegalStateException::class)
    fun stop()

    /**
     * Pauses playback. Call start() to play.
     *
     * @throws IllegalStateException if the internal player engine has not been
     * initialized.
     */
    @Throws(IllegalStateException::class)
    fun pause()

    /**
     * Pauses playback. Call play() to play.
     *
     * @throws IllegalStateException if the internal player engine has not been
     * initialized.
     */
    @Throws(IllegalStateException::class)
    fun play()

    /**
     * Set the low-level power management behavior for this MediaPlayer.  This
     * can be used when the MediaPlayer is not playing through a SurfaceHolder
     * set with [.setDisplay] and thus can use the
     * high-level [.setScreenOnWhilePlaying] feature.
     *
     *
     * This function has the MediaPlayer access the low-level power manager
     * service to control the device's power usage while playing is occurring.
     * The parameter is a combination of [android.os.PowerManager] wake flags.
     * Use of this method requires [android.Manifest.permission.WAKE_LOCK]
     * permission.
     * By default, no attempt is made to keep the device awake during playback.
     *
     * @param context the Context to use
     * @param mode    the power/wake mode to set
     * @see android.os.PowerManager
     */
    fun setWakeMode(context: Context, mode: Int)

    /**
     * Control whether we should use the attached SurfaceHolder to keep the
     * screen on while video playback is occurring.  This is the preferred
     * method over [.setWakeMode] where possible, since it doesn't
     * require that the application have permission for low-level wake lock
     * access.
     *
     * @param screenOn Supply true to keep the screen on, false to allow it
     * to turn off.
     */
    fun setScreenOnWhilePlaying(screenOn: Boolean)

    /**
     * Seeks to specified time position.
     *
     * @param msec the offset in milliseconds from the start to seek to
     * @throws IllegalStateException if the internal player engine has not been
     * initialized
     */
    @Throws(IllegalStateException::class)
    fun seekTo(msec: Float)

    /**
     * Releases resources associated with this MediaPlayer object.
     * It is considered good practice to call this method when you're
     * done using the MediaPlayer. In particular, whenever an Activity
     * of an application is paused (its onPause() method is called),
     * or stopped (its onStop() method is called), this method should be
     * invoked to destroy the MediaPlayer object, unless the application
     * has a special need to keep the object around. In addition to
     * unnecessary resources (such as memory and instances of codecs)
     * being held, failure to call this method immediately if a
     * MediaPlayer object is no longer needed may also lead to
     * continuous battery consumption for mobile devices, and playback
     * failure for other applications if no multiple instances of the
     * same codec are supported on a device. Even if multiple instances
     * of the same codec are supported, some performance degradation
     * may be expected when unnecessary multiple instances are used
     * at the same time.
     */
    fun release()

    /**
     * Resets the MediaPlayer to its uninitialized state. After calling
     * this method, you will have to initialize it again by setting the
     * data source and calling prepare().
     */
    fun reset()

    /**
     * Sets the audio stream type for this MediaPlayer. See [AudioManager]
     * for a list of stream types. Must call this method before prepare() or
     * prepareAsync() in order for the target stream type to become effective
     * thereafter.
     *
     * @param streamtype the audio stream type
     * @see android.media.AudioManager
     */
    fun setAudioStreamType(streamtype: Int)

    /**
     * Sets the volume on this player.
     * This API is recommended for balancing the output of audio streams
     * within an application. Unless you are writing an application to
     * control user settings, this API should be used in preference to
     * [AudioManager.setStreamVolume] which sets the volume of ALL streams of
     * a particular type. Note that the passed volume values are raw scalars.
     * UI controls should be scaled logarithmically.
     *
     * @param leftVolume left volume scalar
     * @param rightVolume right volume scalar
     */
    fun setVolume(leftVolume: Float, rightVolume: Float)

    /**
     * Sets mute state on this player.
     * @param mute
     */
    fun setMute(mute: Boolean)

    /**
     * Sets speed on this player.
     * @param rate
     */
    fun setRate(rate: Float)

    /**
     * Sets pitch on this player.
     * @param pitch
     */
    fun setPitch(pitch: Float)

    interface OnListener {

        /**
         * Called when the media file is ready for playback.
         *
         * @param mp the MediaPlayer that is ready for playback
         */
        fun onStarted(mp: IMediaPlayer)

        /**
         * Called when the end of a media source is reached during playback.
         *
         * @param mp the MediaPlayer that reached the end of the file
         */
        fun onCompletion(mp: IMediaPlayer)

        /**
         * Called to indicate an info or a warning.
         *
         * @param mp      the MediaPlayer the info pertains to.
         * @param what    the type of info or warning.
         *
         *  * [.MEDIA_INFO_UNKNOWN]
         *  * [.MEDIA_INFO_VIDEO_TRACK_LAGGING]
         *  * [.MEDIA_INFO_BUFFERING_START]
         *  * [.MEDIA_INFO_BUFFERING_END]
         *  * [.MEDIA_INFO_BAD_INTERLEAVING]
         *  * [.MEDIA_INFO_NOT_SEEKABLE]
         *  * [.MEDIA_INFO_METADATA_UPDATE]
         *
         * @param extra an extra code, specific to the info. Typically
         * implementation dependant.
         * @return True if the method handled the info, false if it didn't.
         * Returning false, or not having an OnErrorListener at all, will
         * cause the info to be discarded.
         */
        fun onInfo(mp: IMediaPlayer, what: Int, extra: Int): Boolean

        /**
         * Called to indicate an error.
         *
         * @param mp      the MediaPlayer the error pertains to
         * @param what    the type of error that has occurred:
         *
         *  * [.MEDIA_ERROR_UNKNOWN]
         *  * [.MEDIA_ERROR_SERVER_DIED]
         *
         * @param extra an extra code, specific to the error. Typically
         * implementation dependant.
         * @return True if the method handled the error, false if it didn't.
         * Returning false, or not having an OnErrorListener at all, will
         * cause the OnCompletionListener to be called.
         */
        fun onError(mp: IMediaPlayer, what: Int, extra: Int): Boolean

        /**
         * Called to indicate an avaliable timed text
         *
         * @param mp             the MediaPlayer associated with this callback
         * @param text           the timed text sample which contains the text
         * needed to be displayed and the display format.
         * {@hide}
         */
        fun onTimedText(mp: IMediaPlayer, text: TimedText?)

        /**
         * Called to indicate the video size
         *
         * The video size (width and height) could be 0 if there was no video,
         * no display surface was set, or the value was not determined yet.
         *
         * @param width     the width of the video
         * @param height    the height of the video
         */
        fun onVideoSizeChanged(mediaPlayer: IMediaPlayer, width: Int, height: Int)

        /**
         * Called to indicate the completion of a seek operation.
         *
         * @param mp the MediaPlayer that issued the seek operation
         */
        fun onSeekComplete(mp: IMediaPlayer)

        /**
         * Called to update status in buffering a media stream received through
         * progressive HTTP download. The received buffering percentage
         * indicates how much of the content has been buffered or played.
         * For example a buffering update of 80 percent when half the content
         * has already been played indicates that the next 30 percent of the
         * content to play has been buffered.
         *
         * @param mp      the MediaPlayer the update pertains to
         * @param percent the percentage (0-100) of the content
         * that has been buffered or played thus far
         */
        fun onBufferingUpdate(mp: IMediaPlayer, percent: Int)

        fun onCurrentPosition(current: Long, duration: Long)
    }

    fun setOnListener(listener: OnListener)

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