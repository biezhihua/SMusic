package com.bzh.splayer.lib.opengl

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet

class SGLSurfaceView @JvmOverloads constructor(context: Context, attrs: AttributeSet? = null) :
    GLSurfaceView(context, attrs) {

    val render: SRender?

    init {
        setEGLContextClientVersion(2)
        render = SRender(context)
        setRenderer(render)
        renderMode = RENDERMODE_WHEN_DIRTY
        render.renderListener = object : SRender.OnRenderListener {
            override fun onRender() {
                requestRender()
            }
        }
    }

    fun updateYUVData(width: Int, height: Int, y: ByteArray, u: ByteArray, v: ByteArray) {
        if (render != null) {
            render.setYUVRenderData(width, height, y, u, v)
            requestRender()
        }
    }
}
