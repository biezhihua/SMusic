#include "GLInputYUV420PFilter.h"

GLInputYUV420PFilter::GLInputYUV420PFilter() {
    for (int i = 0; i < GLES_MAX_PLANE; ++i) {
        inputTextureHandle[i] = 0;
        textures[i] = 0;
    }
}

GLInputYUV420PFilter::~GLInputYUV420PFilter() {

}

void GLInputYUV420PFilter::initProgram() {
    initProgram(kDefaultVertexShader.c_str(), kYUV420PFragmentShader.c_str());
}

void GLInputYUV420PFilter::initProgram(const char *vertexShader, const char *fragmentShader) {
    if (vertexShader && fragmentShader) {
        programHandle = OpenGLUtils::createProgram(vertexShader, fragmentShader);
        OpenGLUtils::checkGLError("createProgram");
        positionHandle = glGetAttribLocation((GLuint) (programHandle), "aPosition");
        texCoordinateHandle = glGetAttribLocation((GLuint) (programHandle), "aTextureCoord");
        inputTextureHandle[0] = glGetUniformLocation((GLuint) (programHandle), "inputTextureY");
        inputTextureHandle[1] = glGetUniformLocation((GLuint) (programHandle), "inputTextureU");
        inputTextureHandle[2] = glGetUniformLocation((GLuint) (programHandle), "inputTextureV");

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glUseProgram((GLuint) (programHandle));

        if (textures[0] == 0) {
            glGenTextures(3, textures);
        }

        for (int i = 0; i < 3; ++i) {
            glActiveTexture((GLenum) (GL_TEXTURE0 + i));
            glBindTexture(GL_TEXTURE_2D, textures[i]);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glUniform1i(inputTextureHandle[i], i);
        }
        setInitialized(true);
    } else {
        positionHandle = -1;
        positionHandle = -1;
        inputTextureHandle[0] = -1;
        setInitialized(false);
    }
}

GLboolean GLInputYUV420PFilter::uploadTexture(Texture *texture) {
    // 需要设置4字节对齐
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glUseProgram((GLuint) programHandle);

    // 更新绑定纹理的数据
    const GLsizei heights[3] = {texture->height, texture->height / 2, texture->height / 2};
    for (int i = 0; i < 3; ++i) {
        glActiveTexture((GLenum) GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_LUMINANCE,
                     texture->pitches[i],
                     heights[i],
                     0,
                     GL_LUMINANCE,
                     GL_UNSIGNED_BYTE,
                     texture->pixels[i]);
        glUniform1i(inputTextureHandle[i], i);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 0);
    return GL_TRUE;
}

GLboolean GLInputYUV420PFilter::renderTexture(Texture *texture,
                                              float *vertices, float *textureVertices) {
    if (!isInitialized() || !texture) {
        return GL_FALSE;
    }
    // 绑定属性值
    bindAttributes(vertices, textureVertices);
    // 绘制前处理
    onDrawBegin();
    // 绘制纹理
    onDrawFrame();
    // 绘制后处理
    onDrawAfter();
    // 解绑属性
    unbindAttributes();
    // 解绑纹理
    unbindTextures();
    // 解绑program
    glUseProgram(0);
    return GL_TRUE;
}

