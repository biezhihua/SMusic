package com.bzh.splayer

import android.graphics.Rect

class TimedText(bounds: Rect, text: String) : ITimedText {

    override fun onTimedText(bounds: Rect, text: String) {
        this.bounds = bounds
        this.text = text
    }

    var bounds: Rect? = null
    var text: String? = null

}
