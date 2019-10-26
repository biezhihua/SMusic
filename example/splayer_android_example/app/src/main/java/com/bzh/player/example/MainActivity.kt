package com.bzh.player.example

import android.content.pm.PackageManager
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.bzh.player.R
import com.bzh.splayer.IMediaPlayer
import com.bzh.splayer.MediaPlayer
import java.io.File


@Suppress("UNUSED_PARAMETER")
class MainActivity : AppCompatActivity() {

    //    private lateinit var time1: TextView
//    private lateinit var time2: TextView
//    private lateinit var seek: SeekBar
//    private lateinit var volume: SeekBar
//    private lateinit var speed: SeekBar
//    private lateinit var pitch: SeekBar
    private lateinit var surfaceView: SurfaceView
    private lateinit var mediaPlayer: MediaPlayer
    private var mHolder: SurfaceHolder? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
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
        mediaPlayer.setDisplay(mHolder)
    }

    fun setSource(v: View) {
        val file = File(
            Environment.getExternalStorageDirectory(),
            "/Download/The.Walking.Dead.S04E15.2013.BluRay.720p.x264.AC3-CMCT.mkv"
        )
        mediaPlayer.setDataSource(file.absolutePath)
    }

    fun start(v: View) {
        mediaPlayer.setOnErrorListener(object : IMediaPlayer.OnErrorListener {
            override fun onError(mp: IMediaPlayer, what: Int, extra: Int): Boolean {
                Log.d(TAG, "onError() called with: mp = [$mp], what = [$what], extra = [$extra]")
                return false
            }
        })
        mediaPlayer.setOnCompletionListener(object : IMediaPlayer.OnCompletionListener {
            override fun onCompletion(mp: IMediaPlayer) {
                Log.d(TAG, "onCompletion() called with: mp = [$mp]")
            }
        })
        mediaPlayer.prepareAsync()
    }

    fun play(v: View) {
        mediaPlayer.play()
    }

    fun pause(v: View) {
        mediaPlayer.pause()
    }

    fun stop(v: View) {
        mediaPlayer.stop()
    }

    fun destroy(v: View) {
        mediaPlayer.release()
    }

    fun left(v: View) {
    }

    fun right(v: View) {
    }

    fun center(v: View) {
    }

    override fun onResume() {
        super.onResume()
//        mediaPlayer.play();
    }

    override fun onPause() {
        super.onPause()
//        mediaPlayer.pause();
    }

    companion object {
        private const val TAG = "MainActivity"
    }
}
