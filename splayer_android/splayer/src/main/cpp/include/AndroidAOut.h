#ifndef SPLAYER_ANDROIDAOUT_H
#define SPLAYER_ANDROIDAOUT_H

#include <AOut.h>
#include <Common.h>

class AndroidAOut : public AOut {

public:
    AndroidAOut();

    ~AndroidAOut();

    int open() override;

    void pause() override;

    void flush() override;

    void close() override;

    void free() override;
};


#endif //SPLAYER_ANDROIDAOUT_H
