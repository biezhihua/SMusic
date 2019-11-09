#include "Log.h"

#ifdef __ANDROID__
#elif SPLAYER_COMMAND
Mutex *LOG_MUTEX = new Mutex();
#else
#endif

