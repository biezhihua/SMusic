package com.bzh.splayer.opengl

import android.content.Context
import android.graphics.SurfaceTexture
import android.opengl.GLES11Ext
import android.opengl.GLES20
import android.opengl.GLSurfaceView
import android.view.Surface
import com.bzh.splayer.R
import com.bzh.splayer.common.RawResourceReader
import com.bzh.splayer.common.ShaderHelper
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.FloatBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class SRender(private val context: Context) : GLSurfaceView.Renderer {

    enum class RenderType {
        MEDIA,
        SOFT
    }

    var renderType: RenderType =
        RenderType.SOFT

    var renderListener: OnRenderListener? = null

    var surfaceListener: OnSurfaceListener? = null
        set(value) {
            value?.onSurfaceCreate(mediaSurface)
            field = value
        }

    private val vertexBuffer: FloatBuffer
    private val textureBuffer: FloatBuffer

    // media
    private var mediaProgram: Int = 0
    private var mediaAvPositionAttribute: Int = 0
    private var mediaAfPositionAttribute: Int = 0
    private var mediaSamplerExternalOESAttribute: Int = 0
    private var mediaTextureId: Int = 0
    private var mediaSurfaceTexture: SurfaceTexture? = null
    private var mediaSurface: Surface? = null

    // soft
    private var softProgram: Int = 0
    private var softAvPositionAttribute: Int = 0
    private var softAfPositionAttribute: Int = 0

    private var softTextureUniformY: Int = 0
    private var softTextureUniformU: Int = 0
    private var softTextureUniformV: Int = 0
    private var softTextureHandle: IntArray? = null

    private var softWidthYUV: Int = 0
    private var softHeightYUV: Int = 0
    private var softY: ByteBuffer? = null
    private var softU: ByteBuffer? = null
    private var softV: ByteBuffer? = null

    init {
        val vertexData = floatArrayOf(-1f, -1f, 1f, -1f, -1f, 1f, 1f, 1f)

        val textureData = floatArrayOf(0f, 1f, 1f, 1f, 0f, 0f, 1f, 0f)

        vertexBuffer = ByteBuffer.allocateDirect(vertexData.size * 4)
            .order(ByteOrder.nativeOrder())
            .asFloatBuffer()
            .put(vertexData)
        vertexBuffer.position(0)

        textureBuffer = ByteBuffer.allocateDirect(textureData.size * 4)
            .order(ByteOrder.nativeOrder())
            .asFloatBuffer()
            .put(textureData)
        textureBuffer.position(0)
    }

    override fun onSurfaceCreated(gl: GL10, config: EGLConfig) {
        initSoftRenderYUV()
        initMediaRender()
    }

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
        GLES20.glViewport(0, 0, width, height)
        initSoftRenderYUV()
    }

    override fun onDrawFrame(gl: GL10) {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT)
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f)
        if (renderType == RenderType.SOFT) {
            renderSoft()
        } else {
            renderMedia()
        }
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4)
    }


    private fun initSoftRenderYUV() {
        val vertexSource =
            RawResourceReader.readTextFileFromRawResource(context, R.raw.vertex_shader)
        val fragmentSource =
            RawResourceReader.readTextFileFromRawResource(context, R.raw.fragment_softcodec)

        if (vertexSource == null || fragmentSource == null) {
            return
        }
        val vertexShaderHandle = ShaderHelper.compileShader(GLES20.GL_VERTEX_SHADER, vertexSource)
        val fragmentShaderHandle =
            ShaderHelper.compileShader(GLES20.GL_FRAGMENT_SHADER, fragmentSource)

        softProgram = ShaderHelper.createAndLinkProgram(
            vertexShaderHandle, fragmentShaderHandle,
            arrayOf("av_Position", "af_Position")
        )

        softAvPositionAttribute = GLES20.glGetAttribLocation(softProgram, "av_Position")
        softAfPositionAttribute = GLES20.glGetAttribLocation(softProgram, "af_Position")

        softTextureUniformY = GLES20.glGetUniformLocation(softProgram, "texture_y")
        softTextureUniformU = GLES20.glGetUniformLocation(softProgram, "texture_u")
        softTextureUniformV = GLES20.glGetUniformLocation(softProgram, "texture_v")

        softTextureHandle = IntArray(3)
        GLES20.glGenTextures(3, softTextureHandle, 0)

        for (i in 0..2) {
            // Bind to the texture in OpenGL
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, softTextureHandle!![i])

            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT)
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT)
            // Set filtering
            GLES20.glTexParameteri(
                GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_MIN_FILTER,
                GLES20.GL_LINEAR
            )
            GLES20.glTexParameteri(
                GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_MAG_FILTER,
                GLES20.GL_LINEAR
            )
        }
    }

    private fun initMediaRender() {
        val vertexSource =
            RawResourceReader.readTextFileFromRawResource(context, R.raw.vertex_shader)
        val fragmentSource =
            RawResourceReader.readTextFileFromRawResource(context, R.raw.fragment_mediacodec)

        if (vertexSource == null || fragmentSource == null) {
            return
        }
        val vertexShaderHandle = ShaderHelper.compileShader(GLES20.GL_VERTEX_SHADER, vertexSource)
        val fragmentShaderHandle =
            ShaderHelper.compileShader(GLES20.GL_FRAGMENT_SHADER, fragmentSource)

        mediaProgram = ShaderHelper.createAndLinkProgram(
            vertexShaderHandle, fragmentShaderHandle,
            arrayOf("av_Position", "af_Position")
        )

        mediaAvPositionAttribute = GLES20.glGetAttribLocation(mediaProgram, "av_Position")
        mediaAfPositionAttribute = GLES20.glGetAttribLocation(mediaProgram, "af_Position")
        mediaSamplerExternalOESAttribute = GLES20.glGetUniformLocation(mediaProgram, "sTexture")

        val textureIds = IntArray(1)
        GLES20.glGenTextures(1, textureIds, 0)
        mediaTextureId = textureIds[0]

        GLES20.glTexParameteri(
            GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
            GLES20.GL_TEXTURE_WRAP_S,
            GLES20.GL_REPEAT
        )
        GLES20.glTexParameteri(
            GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
            GLES20.GL_TEXTURE_WRAP_T,
            GLES20.GL_REPEAT
        )
        GLES20.glTexParameteri(
            GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
            GLES20.GL_TEXTURE_MIN_FILTER,
            GLES20.GL_LINEAR
        )
        GLES20.glTexParameteri(
            GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
            GLES20.GL_TEXTURE_MAG_FILTER,
            GLES20.GL_LINEAR
        )

        mediaSurfaceTexture = SurfaceTexture(mediaTextureId)
        mediaSurface = Surface(mediaSurfaceTexture)
        mediaSurfaceTexture?.setOnFrameAvailableListener {
            renderListener?.onRender()
        }
        surfaceListener?.onSurfaceCreate(mediaSurface)
    }

    fun setYUVRenderData(width: Int, height: Int, y: ByteArray, u: ByteArray, v: ByteArray) {
        this.softWidthYUV = width
        this.softHeightYUV = height
        this.softY = ByteBuffer.wrap(y)
        this.softU = ByteBuffer.wrap(u)
        this.softV = ByteBuffer.wrap(v)
    }

    private fun renderMedia() {
        if (mediaSurfaceTexture != null) {
            mediaSurfaceTexture?.updateTexImage()
            GLES20.glUseProgram(mediaProgram)

            GLES20.glEnableVertexAttribArray(mediaAvPositionAttribute)
            GLES20.glVertexAttribPointer(
                mediaAvPositionAttribute,
                2,
                GLES20.GL_FLOAT,
                false,
                8,
                vertexBuffer
            )

            GLES20.glEnableVertexAttribArray(mediaAfPositionAttribute)
            GLES20.glVertexAttribPointer(
                mediaAfPositionAttribute,
                2,
                GLES20.GL_FLOAT,
                false,
                8,
                textureBuffer
            )

            GLES20.glActiveTexture(GLES20.GL_TEXTURE0)
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, mediaTextureId)
            GLES20.glUniform1i(mediaSamplerExternalOESAttribute, 0)
        }
    }


    private fun renderSoft() {
        if (softTextureHandle != null && softWidthYUV > 0 && softHeightYUV > 0 && softY != null && softU != null && softV != null) {
            GLES20.glUseProgram(softProgram)

            GLES20.glEnableVertexAttribArray(softAvPositionAttribute)
            GLES20.glVertexAttribPointer(
                softAvPositionAttribute,
                2,
                GLES20.GL_FLOAT,
                false,
                8,
                vertexBuffer
            )

            GLES20.glEnableVertexAttribArray(softAfPositionAttribute)
            GLES20.glVertexAttribPointer(
                softAfPositionAttribute,
                2,
                GLES20.GL_FLOAT,
                false,
                8,
                textureBuffer
            )

            GLES20.glActiveTexture(GLES20.GL_TEXTURE0)
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, softTextureHandle!![0])
            GLES20.glTexImage2D(
                GLES20.GL_TEXTURE_2D,
                0,
                GLES20.GL_LUMINANCE,
                softWidthYUV,
                softHeightYUV,
                0,
                GLES20.GL_LUMINANCE,
                GLES20.GL_UNSIGNED_BYTE,
                softY
            )

            GLES20.glActiveTexture(GLES20.GL_TEXTURE1)
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, softTextureHandle!![1])
            GLES20.glTexImage2D(
                GLES20.GL_TEXTURE_2D,
                0,
                GLES20.GL_LUMINANCE,
                softWidthYUV / 2,
                softHeightYUV / 2,
                0,
                GLES20.GL_LUMINANCE,
                GLES20.GL_UNSIGNED_BYTE,
                softU
            )

            GLES20.glActiveTexture(GLES20.GL_TEXTURE2)
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, softTextureHandle!![2])
            GLES20.glTexImage2D(
                GLES20.GL_TEXTURE_2D,
                0,
                GLES20.GL_LUMINANCE,
                softWidthYUV / 2,
                softHeightYUV / 2,
                0,
                GLES20.GL_LUMINANCE,
                GLES20.GL_UNSIGNED_BYTE,
                softV
            )

            GLES20.glUniform1i(softTextureUniformY, 0)
            GLES20.glUniform1i(softTextureUniformU, 1)
            GLES20.glUniform1i(softTextureUniformV, 2)

            softY?.clear()
            softU?.clear()
            softV?.clear()
            softY = null
            softU = null
            softV = null
        }
    }

    interface OnSurfaceListener {
        fun onSurfaceCreate(surface: Surface?)
    }

    interface OnRenderListener {
        fun onRender()
    }
}
