//
// Created by biezhihua on 2019-06-16.
//

#include "../include/PipelineNode.h"

PipelineNode::PipelineNode() {
    ALOGD(__func__);
    mutex = new Mutex();
}

PipelineNode::~PipelineNode() {
    ALOGD(__func__);
    delete mutex;
}
