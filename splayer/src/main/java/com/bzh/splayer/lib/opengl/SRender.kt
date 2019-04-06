package com.bzh.splayer.lib.opengl

import android.content.Context
import android.opengl.GLES20
import android.opengl.GLSurfaceView
import com.bzh.splayer.lib.R
import com.bzh.splayer.lib.common.RawResourceReader
import com.bzh.splayer.lib.common.ShaderHelper
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.FloatBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class SRender(private val context: Context) : GLSurfaceView.Renderer {

    private val vertexBuffer: FloatBuffer
    private val textureBuffer: FloatBuffer
    private var program: Int = 0
    private var avPositionAttribute: Int = 0
    private var afPositionAttribute: Int = 0
    private val textureId: Int = 0

    private var textureUniformY: Int = 0
    private var textureUniformU: Int = 0
    private var textureUniformV: Int = 0
    private var textureHandle: IntArray? = null

    private var widthYUV: Int = 0
    private var heightYUV: Int = 0
    private var y: ByteBuffer? = null
    private var u: ByteBuffer? = null
    private var v: ByteBuffer? = null

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
        initRenderYUV()
    }

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
        GLES20.glViewport(0, 0, width, height)
        initRenderYUV()
    }

    override fun onDrawFrame(gl: GL10) {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT)
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f)
        renderYUV()
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4)
    }

    private fun initRenderYUV() {
        val vertexSource = RawResourceReader.readTextFileFromRawResource(context, R.raw.vertex_shader)
        val fragmentSource = RawResourceReader.readTextFileFromRawResource(context, R.raw.fragment_shader)

        if (vertexSource == null || fragmentSource == null) {
            return
        }
        val vertexShaderHandle = ShaderHelper.compileShader(GLES20.GL_VERTEX_SHADER, vertexSource)
        val fragmentShaderHandle = ShaderHelper.compileShader(GLES20.GL_FRAGMENT_SHADER, fragmentSource)

        program = ShaderHelper.createAndLinkProgram(
            vertexShaderHandle, fragmentShaderHandle,
            arrayOf("av_Position", "af_Position")
        )

        avPositionAttribute = GLES20.glGetAttribLocation(program, "av_Position")
        afPositionAttribute = GLES20.glGetAttribLocation(program, "af_Position")

        textureUniformY = GLES20.glGetUniformLocation(program, "sampler_y")
        textureUniformU = GLES20.glGetUniformLocation(program, "sampler_u")
        textureUniformV = GLES20.glGetUniformLocation(program, "sampler_v")

        textureHandle = IntArray(3)
        GLES20.glGenTextures(3, textureHandle, 0)

        for (i in 0..2) {
            // Bind to the texture in OpenGL
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureHandle!![i])

            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT)
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT)
            // Set filtering
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR)
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR)
        }
    }

    fun setYUVRenderData(width: Int, height: Int, y: ByteArray, u: ByteArray, v: ByteArray) {
        this.widthYUV = width
        this.heightYUV = height
        this.y = ByteBuffer.wrap(y)
        this.u = ByteBuffer.wrap(u)
        this.v = ByteBuffer.wrap(v)
    }

    private fun renderYUV() {
        if (textureHandle != null && widthYUV > 0 && heightYUV > 0 && y != null && u != null && v != null) {
            GLES20.glUseProgram(program)

            GLES20.glEnableVertexAttribArray(avPositionAttribute)
            GLES20.glVertexAttribPointer(avPositionAttribute, 2, GLES20.GL_FLOAT, false, 8, vertexBuffer)

            GLES20.glEnableVertexAttribArray(afPositionAttribute)
            GLES20.glVertexAttribPointer(afPositionAttribute, 2, GLES20.GL_FLOAT, false, 8, textureBuffer)

            GLES20.glActiveTexture(GLES20.GL_TEXTURE0)
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureHandle!![0])
            GLES20.glTexImage2D(
                GLES20.GL_TEXTURE_2D,
                0,
                GLES20.GL_LUMINANCE,
                widthYUV,
                heightYUV,
                0,
                GLES20.GL_LUMINANCE,
                GLES20.GL_UNSIGNED_BYTE,
                y
            )

            GLES20.glActiveTexture(GLES20.GL_TEXTURE1)
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureHandle!![1])
            GLES20.glTexImage2D(
                GLES20.GL_TEXTURE_2D,
                0,
                GLES20.GL_LUMINANCE,
                widthYUV / 2,
                heightYUV / 2,
                0,
                GLES20.GL_LUMINANCE,
                GLES20.GL_UNSIGNED_BYTE,
                u
            )

            GLES20.glActiveTexture(GLES20.GL_TEXTURE2)
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureHandle!![2])
            GLES20.glTexImage2D(
                GLES20.GL_TEXTURE_2D,
                0,
                GLES20.GL_LUMINANCE,
                widthYUV / 2,
                heightYUV / 2,
                0,
                GLES20.GL_LUMINANCE,
                GLES20.GL_UNSIGNED_BYTE,
                v
            )

            GLES20.glUniform1i(textureUniformY, 0)
            GLES20.glUniform1i(textureUniformU, 1)
            GLES20.glUniform1i(textureUniformV, 2)

            y?.clear()
            u?.clear()
            v?.clear()
            y = null
            u = null
            v = null
        }
    }
}
