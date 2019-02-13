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
        music?.init()
    }

    fun destroyInstance(v: View) {
        music?.release()
        music = null
    }

    fun mainThreadCallJava(v: View) {
    }

    fun subThreadCallJava(v: View) {
    }

    fun mainThreadCallStaticJava(v: View) {
    }

    fun subThreadCallStaticJava(v: View) {
    }

    fun playMp3(v: View) {
        music?.start()
    }

    fun prepareMp3(v: View) {
        music?.setListener(object : IMusicListener {
            override fun onPrepared() {
                Log.d(TAG, "onPrepared() called")
            }
        })
        music?.setDataSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3")
        music?.prepare()
    }

    companion object {
        private const val TAG = "MainActivity"
    }
}
