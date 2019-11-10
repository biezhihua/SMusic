package com.bzh.player.example

import android.content.pm.PackageManager
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import android.widget.SeekBar
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.bzh.player.R
import com.bzh.splayer.IMediaPlayer
import com.bzh.splayer.ITimedText
import com.bzh.splayer.MediaPlayer
import java.io.File


@Suppress("UNUSED_PARAMETER")
class MainActivity : AppCompatActivity() {

    private lateinit var time1: TextView
    private lateinit var time2: TextView
    private lateinit var seek: SeekBar
    private lateinit var volume: SeekBar
    private lateinit var speed: SeekBar
    private lateinit var pitch: SeekBar
    private lateinit var surfaceView: SurfaceView
    private var mediaPlayer: MediaPlayer? = null
    private var mHolder: SurfaceHolder? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        time1 = findViewById(R.id.time1)
        time2 = findViewById(R.id.time2)
        seek = findViewById(R.id.seek)
        volume = findViewById(R.id.volume)
        speed = findViewById(R.id.speed)
        pitch = findViewById(R.id.pitch)
        surfaceView = findViewById(R.id.surfaceView)
        surfaceView.holder.addCallback(object : SurfaceHolder.Callback {
            override fun surfaceChanged(
                holder: SurfaceHolder?,
                format: Int,
                width: Int,
                height: Int
            ) {
                Log.d(
                    TAG,
                    "surfaceChanged() called with: holder = [$holder], format = [$format], width = [$width], height = [$height]"
                )
            }

            override fun surfaceDestroyed(holder: SurfaceHolder?) {
                Log.d(TAG, "surfaceDestroyed() called with: holder = [$holder]")
            }

            override fun surfaceCreated(holder: SurfaceHolder?) {
                Log.d(TAG, "surfaceCreated() called with: holder = [$holder]")
                mHolder = holder
            }
        })

        speed.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onStartTrackingTouch(seekBar: SeekBar?) {

            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
            }

            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
//                mediaPlayer?.speed((progress * 2.0F / 100).toDouble())
            }

        })

        pitch.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onStartTrackingTouch(seekBar: SeekBar?) {

            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
            }

            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                mediaPlayer?.setPitch((progress * 2.0F / 100).toFloat())
            }
        })

        seek.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {

            var mProgress: Int = 0

            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                if (fromUser) {
                    mProgress = progress
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                mediaPlayer?.seekTo(mProgress.toFloat())
            }

        })

        volume.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {

            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                mediaPlayer?.setVolume(progress.toFloat(), progress.toFloat())
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
            }
        })
    }

    override fun onDestroy() {
        super.onDestroy()
        mediaPlayer?.release()
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<String>, grantResults: IntArray
    ) {
        when (requestCode) {
            1 -> {
                // If request is cancelled, the result arrays are empty.
                if (grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {

                    // permission was granted, yay! Do the
                    // contacts-related task you need to do.

                } else {

                    // permission denied, boo! Disable the
                    // functionality that depends on this permission.
                }
                return
            }
        }// other 'case' lines to check for other
        // permissions this app might request
    }

    fun create(v: View) {
        mediaPlayer = MediaPlayer()
        mediaPlayer?.setDisplay(mHolder)
        mediaPlayer?.setOnListener(object : IMediaPlayer.IOnListener {
            override fun onCompletion(mp: IMediaPlayer) {
                Log.d(TAG, "onCompletion() called with: mp = [$mp]")
            }

            override fun onInfo(mp: IMediaPlayer, what: Int, extra: Int): Boolean {
                Log.d(TAG, "onInfo() called with: mp = [$mp], what = [$what], extra = [$extra]")
                return true
            }

            override fun onError(mp: IMediaPlayer, what: Int, extra: Int): Boolean {
                Log.d(TAG, "onError() called with: mp = [$mp], what = [$what], extra = [$extra]")
                return true
            }

            override fun onTimedText(mp: IMediaPlayer, text: ITimedText?) {
                Log.d(TAG, "onTimedText() called with: mp = [$mp], text = [$text]")
            }

            override fun onVideoSizeChanged(mediaPlayer: IMediaPlayer, width: Int, height: Int) {
                Log.d(
                    TAG,
                    "onVideoSizeChanged() called with: mediaPlayer = [$mediaPlayer], width = [$width], height = [$height]"
                )
                val viewWidth = surfaceView.width
                val viewHeight = (viewWidth * height * 1.0F / width).toInt()
                val layoutParams = surfaceView.layoutParams
                layoutParams.width = viewWidth
                layoutParams.height = viewHeight
                surfaceView.layoutParams = layoutParams
            }

            override fun onSeekComplete(mp: IMediaPlayer) {
                Log.d(TAG, "onSeekComplete() called with: mp = [$mp]")
            }

            override fun onBufferingUpdate(mp: IMediaPlayer, percent: Int) {
                Log.d(TAG, "onBufferingUpdate() called with: mp = [$mp], percent = [$percent]")
            }

            override fun onCurrentPosition(current: Long, duration: Long) {
                Log.d(
                    TAG,
                    "onCurrentPosition() called with: current = [$current], duration = [$duration]"
                )
            }

            override fun onStarted(mp: IMediaPlayer) {
                Log.d(
                    TAG,
                    "onStarted() called with: mp = [$mp] width=[${mp.videoWidth}] height=[${mp.videoHeight}]"
                )
            }

        })
    }

    fun setSource(v: View) {
        val file = File(
            Environment.getExternalStorageDirectory(),
            "/Download/The.Walking.Dead.S04E15.2013.BluRay.720p.x264.AC3-CMCT.mkv"
        )
        mediaPlayer?.setDataSource(file.absolutePath)
    }

    fun start(v: View) {
        mediaPlayer?.start()
    }

    fun play(v: View) {
        mediaPlayer?.play()
    }

    fun pause(v: View) {
        mediaPlayer?.pause()
    }

    fun stop(v: View) {
        mediaPlayer?.stop()
    }

    fun destroy(v: View) {
        mediaPlayer?.release()
    }

    fun left(v: View) {
    }

    fun right(v: View) {
    }

    fun center(v: View) {
    }

    override fun onResume() {
        super.onResume()
        mediaPlayer?.play()
    }

    override fun onPause() {
        super.onPause()
        mediaPlayer?.pause()
    }

    companion object {
        private const val TAG = "[MP][UI][MainActivity]"
    }
}
