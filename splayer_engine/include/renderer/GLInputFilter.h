#ifndef ENGINE_GLINPUTFILTER_H
#define ENGINE_GLINPUTFILTER_H

#include <cstdint>

#define GLES_MAX_PLANE 3

/**
 * 纹理图像格式
 */
typedef enum {
    FMT_NONE = -1,
    FMT_ARGB,
    FMT_RGB8,
    FMT_RGB444,
    FMT_RGB555,
    FMT_BGR555,
    FMT_RGB565,
    FMT_BGR565,
    FMT_RGB24,
    FMT_BGR24,
    FMT_0RGB32,
    FMT_0BGR32,
    FMT_NE_RGBX,
    FMT_NE_BGRX,
    FMT_RGB32,
    FMT_RGB32_1,
    FMT_BGR32,
    FMT_BGR32_1,
    FMT_YUV420P,
    FMT_YUYV422,
    FMT_UYVY422,
    FMT_YUVJ420P,
} TextureFormat;

/**
 * 设置翻转模式
 */
typedef enum {
    FLIP_NONE = 0x00,
    FLIP_HORIZONTAL = 0x01,
    FLIP_VERTICAL = 0x02
} FlipDirection;

/**
 * 设置混合模式
 */
typedef enum {

    /// no blending dstRGBA = srcRGBA
            BLEND_NONE = 0x00,

    /// alpha blending
    /// dstRGB = (srcRGB * srcA) + (dstRGB * (1-srcA))
    /// dstA = srcA + (dstA * (1-srcA))
            BLEND_BLEND = 0x01,

    /// additive blending
    /// dstRGB = (srcRGB * srcA) + dstRGB
            BLEND_ADD = 0x02,

    /// color modulate
    /// dstRGB = srcRGB * dstRGB
    /// dstA = dstA
            BLEND_MOD = 0x04,

    BLEND_INVALID = 0x7FFFFFFF

} BlendMode;

#define NUM_DATA_POINTERS 3
/**
 * 纹理结构体，用于记录纹理宽高、混合模式、YUV还是RGBA格式数据等
 */
typedef struct Texture {
    int width;                              // 纹理宽度，即linesize的宽度
    int height;                             // 纹理高度, 帧高度
    int frameWidth;                         // 帧宽度
    int frameHeight;                        // 帧高度
    int rotate;                             // 渲染角度
    BlendMode blendMode;                    // 混合模式，主要是方便后续添加字幕渲染之类的。字幕是绘制到图像上的，需要开启混合模式。
    FlipDirection direction;                // 翻转格式
    TextureFormat format;                   // 纹理图像格式
    uint16_t pitches[NUM_DATA_POINTERS];    // 宽对齐
    uint8_t *pixels[NUM_DATA_POINTERS];     // 像素数据

} Texture;


#endif
