package com.bzh.music.example

import android.annotation.SuppressLint
import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.bzh.music.R
import com.bzh.smusic.lib.IMusicListener
import com.bzh.smusic.lib.SMusic
import java.text.SimpleDateFormat
import java.util.*

class MainActivity : AppCompatActivity() {

    private var music: SMusic? = null

    private lateinit var time: TextView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        time = findViewById(R.id.time)
    }

    fun create(v: View) {
        if (music != null) {
            music?.destroy()
        }
        music = SMusic()
        music?.listener = object : IMusicListener {
            @SuppressLint("SimpleDateFormat")
            override fun onTime(totalTime: Long, currentTime: Long) {
                Log.d(TAG, "onTime() called with: totalTime = [$totalTime], currentTime = [$currentTime]")
                val formatter = SimpleDateFormat("mm:ss")
                time.text = formatter.format(Date(totalTime)) + "/" + formatter.format(Date(currentTime))

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

    companion object {
        private const val TAG = "MainActivity"
    }
}
