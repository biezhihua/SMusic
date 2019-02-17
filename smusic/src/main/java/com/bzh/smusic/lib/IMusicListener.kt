package com.bzh.smusic.lib

interface IMusicListener {
    fun onTime(totalTime: Long, currentTime: Long)
}