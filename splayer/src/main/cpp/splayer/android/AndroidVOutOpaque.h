#ifndef SPLAYER_ANDROIDVOUTOPAQUE_H
#define SPLAYER_ANDROIDVOUTOPAQUE_H

#include <android/native_window.h>
#include "../media/VOutOpaque.h"

class AndroidVOutOpaque : public VOutOpaque {

private:
    ANativeWindow   *pNativeWindow;

public:
    AndroidVOutOpaque();

    virtual ~AndroidVOutOpaque();

};


#endif //SPLAYER_ANDROIDVOUTOPAQUE_H
