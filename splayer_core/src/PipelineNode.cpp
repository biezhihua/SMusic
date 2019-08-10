#include "PipelineNode.h"

PipelineNode::PipelineNode() {
    mutex = new Mutex();
}

PipelineNode::~PipelineNode() {
    delete mutex;
}
