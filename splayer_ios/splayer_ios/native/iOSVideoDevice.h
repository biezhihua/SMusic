#ifndef SPLAYER_IOS_VIDEODEVICE_H
#define SPLAYER_IOS_VIDEODEVICE_H

#include "VideoDevice.h"

class iOSVideoDevice : public VideoDevice {

    const char *const TAG = "[MP][NATIVE][iOSVideoDevice]";
    
public:

    iOSVideoDevice();

    ~iOSVideoDevice() override;

    int create() override;

    int destroy() override;

    int onInitTexture(int initTexture, int newWidth, int newHeight, TextureFormat format, BlendMode blendMode, int rotate) override;

    int onUpdateYUV(uint8_t *yData, int yPitch, uint8_t *uData, int uPitch, uint8_t *vData, int vPitch) override;

    int onUpdateARGB(uint8_t *rgba, int pitch) override;

    void onRequestRenderStart(Frame *frame) override;

    int onRequestRenderEnd(Frame *frame, bool flip) override;

    TextureFormat getTextureFormat(int format) override;

    BlendMode getBlendMode(TextureFormat format) override;
};


#endif
