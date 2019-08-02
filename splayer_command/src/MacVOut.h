

#ifndef SPLAYER_COMMAND_MACVOUT_H
#define SPLAYER_COMMAND_MACVOUT_H


#include <VOut.h>

class MacVOut : public VOut {
public:

    VOutOverlay *createOverlay(int width, int height, int frameFormat) override;

    int displayOverlay(VOutOverlay *overlay) override;

    void free() override;
};


#endif //SPLAYER_COMMAND_MACVOUT_H
