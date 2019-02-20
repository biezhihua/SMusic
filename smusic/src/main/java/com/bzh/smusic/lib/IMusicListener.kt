package com.bzh.smusic.lib

interface IMusicListener {
    fun onTime(totalTime: Int, currentTime: Int)
}