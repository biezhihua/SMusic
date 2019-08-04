#ifndef SPLAYER_MUTEX_H
#define SPLAYER_MUTEX_H

#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <sys/time.h>

#define MUTEX_TIMEDOUT  1
#define MUTEX_MAXWAIT   (~(uint32_t)0)

class Mutex {

private:
    pthread_mutex_t mutex;
    pthread_cond_t cond;

public:
    Mutex();

    ~Mutex();

    /**
     * lock a mutex
     */
    int mutexLock();

    /**
     * unlock a mutex
     */
    int mutexUnLock();

    /**
     * Signal a condition
     */
    int condSignal();

    /**
     * Broadcast a condition
     */
    int condBroadcast();

    /**
     * Wait on a condition with timeout
     */
    int condWaitTimeout(uint32_t ms);

    int condWaitTimeout(Mutex *other, uint32_t ms);

    /**
     * Wait on a condition
     */
    int condWait();
};


#endif //SPLAYER_MUTEX_H
