
#ifndef SPLAYER_PIPELINEOPAQUE_H
#define SPLAYER_PIPELINEOPAQUE_H

#include "Log.h"
#include "Mutex.h"
#include "VOut.h"

class PipelineOpaque {

private:

    VOut *pVOut = nullptr;


public:
    PipelineOpaque();

    virtual ~PipelineOpaque();

    void setVOut(VOut *pOut);
};


#endif //SPLAYER_PIPELINEOPAQUE_H
