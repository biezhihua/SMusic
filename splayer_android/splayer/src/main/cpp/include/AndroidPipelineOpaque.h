#ifndef SPLAYER_ANDROIDPIPELINEOPAQUE_H
#define SPLAYER_ANDROIDPIPELINEOPAQUE_H


#include <PipelineOpaque.h>

class AndroidPipelineOpaque : public PipelineOpaque {

private:
    Mutex *pSurfaceMutex = nullptr;
    float leftVolume = 0.0;
    float rightVolume = 0.0;

public:
    AndroidPipelineOpaque();

    virtual ~AndroidPipelineOpaque();
};


#endif //SPLAYER_ANDROIDPIPELINEOPAQUE_H
