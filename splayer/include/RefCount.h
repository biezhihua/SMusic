#ifndef SPLAYER_REFCOUNT_H
#define SPLAYER_REFCOUNT_H
#include "Log.h"

class RefCount {

private:
    volatile int referenceCount = 0;

public:
    RefCount();

    ~RefCount();

    int incrementReference();

    int decrementReference();
};


#endif //SPLAYER_REFCOUNT_H
