package com.bzh.music.example

import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.bzh.music.R
import com.bzh.smusic.lib.SMusic

class MainActivity : AppCompatActivity() {

    private var music: SMusic? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
    }

    fun create(v: View) {
        if (music != null) {
            music?.destroy()
        }
        music = SMusic()
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
