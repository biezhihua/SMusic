package com.bzh.example

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.bzh.smusic.Demo
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    val demo = Demo()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Example of a call to a native method
        sample_text.text = demo.stringFromJNI()
    }

}
