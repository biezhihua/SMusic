#include "CoordinateUtils.h"

static const float vertices_default[] = {
        -1.0F, -1.0F,  // left,  bottom
        1.0F, -1.0F,  // right, bottom
        -1.0F, 1.0F,  // left,  top
        1.0F, 1.0F,  // right, top
};

static const short indices_default[] = {
        0, 1, 2,
        2, 1, 3,
};

static const float texture_vertices_none[] = {
        0.0F, 0.0F, // left,  bottom
        1.0F, 0.0F, // right, bottom
        0.0F, 1.0F, // left,  top
        1.0F, 1.0F, // right, top
};

static const float texture_vertices_none_input[] = {
        0.0F, 1.0F, // left, top
        1.0F, 1.0F, // right, top
        0.0F, 0.0F, // left, bottom
        1.0F, 0.0F, // right, bottom
};

static const float texture_vertices_90[] = {
        1.0F, 0.0F, // right, bottom
        1.0F, 1.0F, // right, top
        0.0F, 0.0F, // left,  bottom
        0.0F, 1.0F, // left,  top
};

static const float texture_vertices_90_input[] = {
        1.0F, 1.0F, // right, top
        1.0F, 0.0F, // right, bottom
        0.0F, 1.0F, // left,  top
        0.0F, 0.0F, // left,  bottom
};

static const float texture_vertices_180[] = {
        1.0F, 1.0F, // righ,  top
        0.0F, 1.0F, // left,  top
        1.0F, 0.0F, // right, bottom
        0.0F, 0.0F, // left,  bottom
};

static const float texture_vertices_180_input[] = {
        1.0F, 0.0F, // right, bottom
        0.0F, 0.0F, // left,  bottom
        1.0F, 1.0F, // right, top
        0.0F, 1.0F, // left,  top
};

static const float texture_vertices_270[] = {
        0.0F, 1.0F, // left,  top
        0.0F, 0.0F, // left,  bottom
        1.0F, 1.0F, // right, top
        1.0F, 0.0F, // right, bottom
};

static const float texture_vertices_270_input[] = {
        0.0F, 0.0F, // left,  bottom
        0.0F, 1.0F, // left,  top
        1.0F, 0.0F, // right, bottom
        1.0F, 1.0F, // right, top
};

static const float texture_vertices_flip_vertical[] = {
        0.0F, 1.0F, // left,  top
        1.0F, 1.0F, // right, top
        0.0F, 0.0F, // left,  bottom
        1.0F, 0.0F, // right, bottom
};

static const float texture_vertices_flip_horizontal[] = {
        1.0F, 0.0F, // right, bottom
        0.0F, 0.0F, // left,  bottom
        1.0F, 1.0F, // right, top
        0.0F, 1.0F, // left,  top
};

const float *CoordinateUtils::getVertexCoordinates() {
    return vertices_default;
}

const short *CoordinateUtils::getDefaultIndices() {
    return indices_default;
}

const float *CoordinateUtils::getTextureCoordinates(const RotationMode &rotationMode) {
    switch (rotationMode) {
        case ROTATE_NONE: {
            return texture_vertices_none;
        }
        case ROTATE_90: {
            return texture_vertices_90;
        }
        case ROTATE_180: {
            return texture_vertices_180;
        }
        case ROTATE_270: {
            return texture_vertices_270;
        }
        case ROTATE_FLIP_VERTICAL: {
            return texture_vertices_flip_vertical;
        }
        case ROTATE_FLIP_HORIZONTAL: {
            return texture_vertices_flip_horizontal;
        }
    }
    return texture_vertices_none;
}


const float *CoordinateUtils::getInputTextureCoordinates(const RotationMode &rotationMode) {
    switch (rotationMode) {
        case ROTATE_NONE: {
            return texture_vertices_none_input;
        }
        case ROTATE_90: {
            return texture_vertices_90_input;
        }
        case ROTATE_180: {
            return texture_vertices_180_input;
        }
        case ROTATE_270: {
            return texture_vertices_270_input;
        }
        case ROTATE_FLIP_VERTICAL: {
            return texture_vertices_flip_vertical;
        }
        case ROTATE_FLIP_HORIZONTAL: {
            return texture_vertices_flip_horizontal;
        }
    }
    return texture_vertices_none_input;
}