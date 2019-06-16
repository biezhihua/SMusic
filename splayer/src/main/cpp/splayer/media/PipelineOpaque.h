
#ifndef SPLAYER_PIPELINEOPAQUE_H
#define SPLAYER_PIPELINEOPAQUE_H

#include "Log.h"
#include "Mutex.h"
#include "VOut.h"

class PipelineOpaque {

private:
    Mutex *pSurfaceMutex = nullptr;
    float leftVolume = 0.0;
    float rightVolume = 0.0;

public:
    PipelineOpaque();

    virtual ~PipelineOpaque();
};


#endif //SPLAYER_PIPELINEOPAQUE_H
