#ifndef SPLAYER_THREAD_H
#define SPLAYER_THREAD_H

#include <pthread.h>
#include <string>
#include "Log.h"
#include <map>

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

    static map<pthread_t, string> threads;

    static const char *getThreadNameById(pthread_t t);
};


#endif //SPLAYER_THREAD_H
