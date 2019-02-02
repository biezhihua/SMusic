package com.bzh.example

import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.bzh.smusic.Demo
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    private val demo = Demo()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Example of a call to a native method
        sample_text.text = demo.stringFromJNI()
        sample_text.setOnClickListener {
            demo.testFFmpeg()
        }
    }

    fun normalThread(v: View) {
        demo.normalThread()
    }

    fun mutexThread(v: View) {
        demo.mutexThread()
    }

    companion object {
        private const val TAG = "MainActivity"
    }
}
