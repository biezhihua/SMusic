#ifndef SPLAYER_COMMAND_MACPIPELINE_H
#define SPLAYER_COMMAND_MACPIPELINE_H

#include "Stream.h"
#include "Error.h"

class MacStream : public Stream {

public:

    int create() override;

    int destroy() override;

};


#endif //SPLAYER_COMMAND_MACPIPELINE_H
