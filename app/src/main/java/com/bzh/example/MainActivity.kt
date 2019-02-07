package com.bzh.example

import android.os.Bundle
import android.util.Log
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.bzh.smusic.IMusicListener
import com.bzh.smusic.SMusic

class MainActivity : AppCompatActivity() {

//    private val demo = Demo()

    private val music = SMusic()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Example of a call to a native method
//        sample_text.text = demo.stringFromJNI()
//        sample_text.setOnClickListener {
//            demo.testFFmpeg()
//        }
        music.nativeInit()
    }

    override fun onDestroy() {
        music.nativeDestroy()
        super.onDestroy()
    }

    fun normalThread(v: View) {
//        demo.normalThread()
    }

    fun mutexThread(v: View) {
//        demo.mutexThread()
    }

    fun mainThreadCallJava(v: View) {
//        demo.mainThreadCallJava()
    }

    fun subThreadCallJava(v: View) {
//        demo.subThreadCallJava()
    }

    fun mainThreadCallStaticJava(v: View) {
//        demo.mainThreadCallStaticJava()
    }

    fun subThreadCallStaticJava(v: View) {
//        demo.subThreadCallStaticJava()
    }

    fun playMp3(v: View) {
        music.start()
    }

    fun prepareMp3(v: View) {
        music.setListener(object : IMusicListener {
            override fun onPrepared() {
                Log.d(TAG, "onPrepared() called")
                music.start()
            }
        })
        music.setDataSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3")
        music.prepare()
    }

    companion object {
        private const val TAG = "MainActivity"
    }
}
