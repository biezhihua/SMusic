#ifndef SPLAYER_COMMAND_MACAOUT_H
#define SPLAYER_COMMAND_MACAOUT_H

#include <Audio.h>

class MacAudio : public Audio {

public:
    int open() override;

    void pause() override;

    void flush() override;

    void close() override;

    void free() override;
};


#endif //SPLAYER_COMMAND_MACAOUT_H
