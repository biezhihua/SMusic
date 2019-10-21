package com.bzh.splayer

import android.graphics.Rect

class TimedText(bounds: Rect, text: String) {

    var bounds: Rect? = null
    var text: String? = null

    init {
        this.bounds = bounds
        this.text = text
    }
}
