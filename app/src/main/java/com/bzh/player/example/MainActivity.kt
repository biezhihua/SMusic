package com.bzh.player.example

import android.Manifest
import android.annotation.SuppressLint
import android.content.pm.PackageManager
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.View
import android.widget.ImageView
import android.widget.SeekBar
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.bumptech.glide.Glide
import com.bzh.player.R
import com.bzh.splayer.lib.IPlayerListener
import com.bzh.splayer.lib.SPlayer
import java.io.File
import java.text.SimpleDateFormat
import java.util.*


class MainActivity : AppCompatActivity() {

    private var music: SPlayer? = null

    private lateinit var time1: TextView
    private lateinit var time2: TextView
    private lateinit var seek: SeekBar
    private lateinit var volume: SeekBar
    private lateinit var speed: SeekBar
    private lateinit var pitch: SeekBar
    private lateinit var image: ImageView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        time1 = findViewById(R.id.time1)
        time2 = findViewById(R.id.time2)
        seek = findViewById(R.id.seek)
        volume = findViewById(R.id.volume)
        speed = findViewById(R.id.speed)
        pitch = findViewById(R.id.pitch)
        image = findViewById(R.id.image)


        speed.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onStartTrackingTouch(seekBar: SeekBar?) {

            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
            }

            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                music?.speed((progress * 2.0F / 100).toDouble())
            }

        })

        pitch.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onStartTrackingTouch(seekBar: SeekBar?) {

            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
            }

            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                music?.pitch((progress * 2.0F / 100).toDouble())
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
                music?.seek(mProgress)
            }

        })

        volume.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {

            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                music?.volume(progress)
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
            }
        })

        // Here, thisActivity is the current activity
        if (ContextCompat.checkSelfPermission(
                this,
                Manifest.permission.READ_EXTERNAL_STORAGE
            ) != PackageManager.PERMISSION_GRANTED
        ) {

            // Should we show an explanation?
            if (ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.READ_EXTERNAL_STORAGE)) {

                // Show an expanation to the user *asynchronously* -- don't block
                // this thread waiting for the user's response! After the user
                // sees the explanation, try again to request the permission.

            } else {

                // No explanation needed, we can request the permission.

                ActivityCompat.requestPermissions(
                    this, arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE),
                    1
                )

                // MY_PERMISSIONS_REQUEST_READ_CONTACTS is an
                // app-defined int constant. The callback method gets the
                // result of the request.
            }
        }

        Glide.with(this)
            .load("http://attimg.dospy.com/img/day_120708/20120708_562d17b32de40740fb9aKkL4ZcIaNhll.jpg")
            .into(image)
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<String>, grantResults: IntArray
    ) {
        when (requestCode) {
            1 -> {
                // If request is cancelled, the result arrays are empty.
                if (grantResults.size > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {

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
        if (music != null) {
            music?.destroy()
        }
        music = SPlayer()
        music?.listener = object : IPlayerListener {
            override fun onStart() {
                if (music != null) {
                    volume.progress = music!!.getCurrentVolumePercent()
                    speed.progress = (music!!.getCurrentSpeed() * 100 / 2).toInt()
                    pitch.progress = (music!!.getCurrentPitch() * 100 / 2).toInt()
                }
            }

            override fun onPlay() {
            }

            override fun onPause() {
            }

            override fun onStop() {
            }

            override fun onComplete() {
            }

            override fun onDestroy() {
            }

            override fun onLoadState(loadState: Boolean) {
            }

            override fun onError(code: Int, message: String) {
            }

            override fun onCreate() {

            }

            @SuppressLint("SimpleDateFormat")
            override fun onTime(totalTime: Int, currentTime: Int) {
                Log.d(TAG, "onTime() called with: totalTime = [$totalTime], currentTime = [$currentTime]")
                val formatter = SimpleDateFormat("mm:ss")
                time1.text = formatter.format(Date(currentTime.toLong()))
                time2.text = formatter.format(Date(totalTime.toLong()))
                seek.max = totalTime
                seek.progress = currentTime
            }
        }
        music?.create()
    }

    fun setSource(v: View) {
        val file = File(Environment.getExternalStorageDirectory(), "/DCIM/cogo.mp4")
//        val file = File(Environment.getExternalStorageDirectory(), "/DCIM/1552562921640202.mp4")
//        music?.setDataSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3")
        music?.setDataSource(file.absolutePath)
//        music?.setDataSource("https://github.com/biezhihua/SPlayer/blob/master/mp4/1552562921640202.mp4")
//        music?.setDataSource("https://storage.googleapis.com/exoplayer-test-media-1/360/congo.mp4")
//        music?.setDataSource("https://storage.googleapis.com/exoplayer-test-media-0/play.mp3")
//        Log.d(TAG, "setSource() called with: isExist = [${file.exists()}]")
//        try {
//            val inputStream = FileInputStream(file)
//            val read = inputStream.read()
//        } catch (e: Exception) {
//            e.printStackTrace()
//        }
    }

    fun start(v: View) {
        music?.start()
    }

    fun play(v: View) {
        music?.play()
    }

    fun pause(v: View) {
        music?.pause()
    }

    fun stop(v: View) {
        music?.stop()
    }

    fun destroy(v: View) {
        music?.destroy()
        music = null
    }

    fun left(v: View) {
        music?.mute(SPlayer.Mute.LEFT)
    }

    fun right(v: View) {
        music?.mute(SPlayer.Mute.RIGHT)
    }

    fun center(v: View) {
        music?.mute(SPlayer.Mute.CENTER)
    }

    override fun onResume() {
        super.onResume()
        music?.play()
    }

    override fun onPause() {
        super.onPause()
        music?.pause()
    }

    companion object {
        private const val TAG = "MainActivity"
    }
}
