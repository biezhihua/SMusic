//
// Created by biezhihua on 2019-06-16.
//

#include "PipelineNode.h"

PipelineNode::PipelineNode() {
    ALOGD(__func__);
    pMutex = new Mutex();
}

PipelineNode::~PipelineNode() {
    ALOGD(__func__);
    delete pMutex;
}
