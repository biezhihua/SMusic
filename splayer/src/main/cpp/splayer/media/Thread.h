#ifndef SPLAYER_THREAD_H
#define SPLAYER_THREAD_H

#include <pthread.h>
#include <string>

using namespace std;

enum ThreadPriority {
    THREAD_PRIORITY_LOW,
    THREAD_PRIORITY_NORMAL,
    THREAD_PRIORITY_HIGH
};

class Thread {

public:
    pthread_t id;

    int (*func)(void *);

    void *data;

    string *name = nullptr;

    int retval;

    Thread(int (*func)(void *), void *data, const char *name);

    ~Thread();

    int waitThread();

    void detachThread();
};


#endif //SPLAYER_THREAD_H
