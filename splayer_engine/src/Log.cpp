#include "Log.h"

#ifdef __ANDROID__
#else
Mutex *LOG_MUTEX = new Mutex();
#endif

bool DEBUG = true;
