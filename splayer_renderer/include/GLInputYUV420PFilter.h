#ifndef RENDERER_GLINPUTYUV420PFILTER_H
#define RENDERER_GLINPUTYUV420PFILTER_H

#include "GLInputFilter.h"
#include <include/OpenGLUtils.h>

const std::string kYUV420PFragmentShader = SHADER_TO_STRING(
        precision mediump float;
        varying highp vec2 textureCoordinate;
        uniform lowp sampler2D inputTextureY;
        uniform lowp sampler2D inputTextureU;
        uniform lowp sampler2D inputTextureV;

        void main() {
            vec3 yuv;
            vec3 rgb;
            yuv.r = texture2D(inputTextureY, textureCoordinate).r - (16.0 / 255.0);
            yuv.g = texture2D(inputTextureU, textureCoordinate).r - 0.5;
            yuv.b = texture2D(inputTextureV, textureCoordinate).r - 0.5;
            rgb = mat3(1.164,  1.164,  1.164,
                       0.0,   -0.213,  2.112,
                       1.793, -0.533,    0.0) * yuv;
            gl_FragColor = vec4(rgb, 1.0);
        }
);

/**
 * YUV420P输入滤镜
 */
class GLInputYUV420PFilter : public GLInputFilter {

    const char *const TAG = "[MP][RENDER][GLInputYUV420PFilter]";

public:
    GLInputYUV420PFilter();

    virtual ~GLInputYUV420PFilter();

    void initProgram() override;

    void initProgram(const char *vertexShader, const char *fragmentShader) override;

    GLboolean renderTexture(Texture *texture, float *vertices, float *textureVertices) override;

    GLboolean uploadTexture(Texture *texture) override;
};


#endif //GLINPUTYUV420PFILTER_H
