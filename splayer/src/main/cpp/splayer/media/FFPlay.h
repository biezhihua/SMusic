#ifndef SPLAYER_PLAY_H
#define SPLAYER_PLAY_H

#include "Log.h"
#include "Mutex.h"
#include "MessageQueue.h"
#include "AOut.h"
#include "VOut.h"
#include "Pipeline.h"

class FFPlay {

private:
    Mutex *pAvMutex = nullptr;
    Mutex *pVfMutex = nullptr;
    MessageQueue *pMsgQueue = nullptr;
    AOut *pAOut = nullptr;
    VOut *pVOut = nullptr;
    Pipeline *pPipeline = nullptr;
public:
    FFPlay();

    ~FFPlay();

    void setAOut(AOut *aOut);

    void setVOut(VOut *vOut);

    VOut *getVOut() const;

    void setPipeline(Pipeline *pipeline);

    Pipeline *getPipeline() const;

};


#endif //SPLAYER_PLAY_H
