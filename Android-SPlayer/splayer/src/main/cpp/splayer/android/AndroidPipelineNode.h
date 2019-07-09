#ifndef SPLAYER_ANDROIDPIPENODE_H
#define SPLAYER_ANDROIDPIPENODE_H

#include <PipelineNode.h>

class AndroidPipelineNode : public PipelineNode {

public:
    AndroidPipelineNode();

    virtual ~AndroidPipelineNode();

    virtual void destroy();

    virtual int runSync();

    virtual int flush();
};


#endif //SPLAYER_ANDROIDPIPENODE_H
