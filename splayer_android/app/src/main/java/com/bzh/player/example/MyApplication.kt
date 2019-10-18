package com.bzh.player.example

import android.app.Application

class MyApplication : Application() {

    private var token: Long? = null

    override fun onCreate() {
        super.onCreate()
        instance = this
    }

    fun getToken(): String {
        return token.toString()
    }

    fun setToken(token: Long?) {
        this.token = token
    }

    companion object {
        var instance: MyApplication? = null
            private set
    }
}
