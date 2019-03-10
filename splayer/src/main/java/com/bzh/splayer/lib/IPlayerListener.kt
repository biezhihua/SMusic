package com.bzh.splayer.lib

interface IPlayerListener {

    fun onCreate()

    fun onStart()

    fun onPlay()

    fun onPause()

    fun onStop()

    fun onComplete()

    fun onDestroy()

    fun onLoadState(loadState: Boolean)

    fun onError(code: Int, message: String)

    fun onTime(totalTime: Int, currentTime: Int)
}