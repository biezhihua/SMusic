#ifndef RENDERER_GLINPUTBGRAFILTER_H
#define RENDERER_GLINPUTBGRAFILTER_H

#include "GLInputFilter.h"

const std::string kABGRFragmentShader = SHADER_TO_STRING(
        precision mediump float;
        uniform sampler2D inputTexture;
        varying vec2 textureCoordinate;

        void main() {
            vec4 abgr = texture2D(inputTexture, textureCoordinate);
            gl_FragColor = abgr;
            gl_FragColor.r = abgr.b;
            gl_FragColor.b = abgr.r;
        }
);


/**
 * BGRA输入滤镜
 */
class GLInputABGRFilter : public GLInputFilter {

    const char *const TAG = "[MP][RENDER][GLInputABGRFilter]";

public:
    GLInputABGRFilter();

    virtual ~GLInputABGRFilter();

    void initProgram() override;

    void initProgram(const char *vertexShader, const char *fragmentShader) override;

    GLboolean renderTexture(Texture *texture, float *vertices, float *textureVertices) override;

    GLboolean uploadTexture(Texture *texture) override;
};


#endif //GLINPUTBGRAFILTER_H
