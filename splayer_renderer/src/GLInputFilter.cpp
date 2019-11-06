#include "GLInputFilter.h"

GLInputFilter::GLInputFilter() {

}

GLInputFilter::~GLInputFilter() {

}

GLboolean GLInputFilter::uploadTexture(Texture *texture) {
    return GL_TRUE;
}

GLboolean GLInputFilter::renderTexture(Texture *texture, float *vertices, float *textureVertices) {
    return GL_TRUE;
}
