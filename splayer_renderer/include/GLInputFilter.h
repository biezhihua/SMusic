#ifndef RENDERER_GLINPUTFILTER_H
#define RENDERER_GLINPUTFILTER_H

#include <cstdint>
#include "Texture.h"
#include "GLFilter.h"

#define GLES_MAX_PLANE 3

#define NUM_DATA_POINTERS 3

/**
 * 图像数据输入滤镜基类
 */
class GLInputFilter : public GLFilter {

    const char *const TAG = "[MP][RENDER][GLInputFilter]";

public:
    GLInputFilter();

    virtual ~GLInputFilter();

    virtual GLboolean uploadTexture(Texture *texture);

    virtual GLboolean renderTexture(Texture *texture, float *vertices, float *textureVertices);

protected:
    GLuint textures[GLES_MAX_PLANE];        // 纹理id
};


#endif //GLINPUTFILTER_H
