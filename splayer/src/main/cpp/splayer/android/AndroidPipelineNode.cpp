//
// Created by biezhihua on 2019-06-16.
//

#include "AndroidPipelineNode.h"

AndroidPipelineNode::AndroidPipelineNode() : PipelineNode() {
    ALOGD(__func__);
}

AndroidPipelineNode::~AndroidPipelineNode() {
    ALOGD(__func__);
}

void AndroidPipelineNode::destroy() {
    ALOGD(__func__);
}

int AndroidPipelineNode::runSync() {
    ALOGD(__func__);
    return 0;
}

int AndroidPipelineNode::flush() {
    ALOGD(__func__);
    return 0;
}
