package com.bzh.smusic

class Demo {

    external fun stringFromJNI(): String

    external fun testFFmpeg()

    external fun normalThread()

    external fun mutexThread()

    companion object {

        init {
            System.loadLibrary("smusic")
            System.loadLibrary("sffmpeg")
        }
    }
}