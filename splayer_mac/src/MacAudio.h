#ifndef SPLAYER_COMMAND_MACAOUT_H
#define SPLAYER_COMMAND_MACAOUT_H

#include "Audio.h"
#include "Error.h"

class MacAudio : public Audio {

public:
    int create() override;

    int destroy() override;
};


#endif //SPLAYER_COMMAND_MACAOUT_H
