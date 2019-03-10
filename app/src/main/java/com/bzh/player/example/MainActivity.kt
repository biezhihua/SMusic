package com.bzh.player.example

import android.annotation.SuppressLint
import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.SeekBar
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.bzh.player.R
import com.bzh.splayer.lib.IPlayerListener
import com.bzh.splayer.lib.SPlayer
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

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        time1 = findViewById(R.id.time1)
        time2 = findViewById(R.id.time2)
        seek = findViewById(R.id.seek)
        volume = findViewById(R.id.volume)
        speed = findViewById(R.id.speed)
        pitch = findViewById(R.id.pitch)

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
        music?.setDataSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3")
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
