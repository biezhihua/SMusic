package com.bzh.music.example

import android.os.Bundle
import android.util.Log
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.bzh.music.R
import com.bzh.smusic.lib.IMusicListener
import com.bzh.smusic.lib.SMusic

class MainActivity : AppCompatActivity() {

    private var music: SMusic? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
    }

    override fun onDestroy() {
        super.onDestroy()
    }

    fun createInstance(v: View) {
        music = SMusic()
        music?.create()
    }

    fun destroyInstance(v: View) {
        music?.release()
        music = null
    }

    fun prepare(v: View) {
        music?.setListener(object : IMusicListener {
            override fun onPrepared() {
                Log.d(TAG, "onPrepared() called")
            }
        })
        music?.setDataSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3")
        music?.prepare()
    }

    fun play(v: View) {
    }

    fun stop(v: View) {
    }

    companion object {
        private const val TAG = "MainActivity"
    }
}
