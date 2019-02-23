package com.bzh.music.example

import android.annotation.SuppressLint
import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.SeekBar
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.bzh.music.R
import com.bzh.smusic.lib.IMusicListener
import com.bzh.smusic.lib.SMusic
import java.text.SimpleDateFormat
import java.util.*

class MainActivity : AppCompatActivity() {

    private var music: SMusic? = null

    private lateinit var time1: TextView
    private lateinit var time2: TextView
    private lateinit var seek: SeekBar

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        time1 = findViewById(R.id.time1)
        time2 = findViewById(R.id.time2)
        seek = findViewById(R.id.seek)

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
    }

    fun create(v: View) {
        if (music != null) {
            music?.destroy()
        }
        music = SMusic()
        music?.listener = object : IMusicListener {
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
