package com.bzh.splayer.lib.opengl

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet

class SGLSurfaceView @JvmOverloads constructor(context: Context, attrs: AttributeSet? = null) :
    GLSurfaceView(context, attrs) {

    private val render: SRender?

    init {
        setEGLContextClientVersion(2)
        render = SRender(context)
        setRenderer(render)
        renderMode = GLSurfaceView.RENDERMODE_WHEN_DIRTY
    }

    fun updateYUVData(width: Int, height: Int, y: ByteArray, u: ByteArray, v: ByteArray) {
        if (render != null) {
            render.setYUVRenderData(width, height, y, u, v)
            requestRender()
        }
    }
}
