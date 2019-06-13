#ifndef SPLAYER_MESSAGEQUEUE_H
#define SPLAYER_MESSAGEQUEUE_H

#include <list>
#include <queue>
#include "Mutex.h"
#include "Message.h"

using namespace std;

class MessageQueue {
private:

    Mutex *pMutex = NULL;

    list<Message *> *pQueue = NULL;

    bool abortRequest = false;

private:
    int _put(Message *msg);

public:

    MessageQueue();

    ~MessageQueue();

    /**
     * Push Message To Queue
     */
    int put(Message *msg);

    /**
     * Get First Message From Queue
     */
    int get(Message *msg, bool block);

    /**
     * Remove Message By What
     */
    void remove(int what);

    /**
     * Abort Message Queue
     */
    void setAbortRequest(bool abortRequest);

    /**
     * Clear Queue
     */
    void clear();

    void start();
};


#endif
