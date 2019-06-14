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
     * push message to queue
     */
    int put(Message *msg);

    /**
     * get first message from queue, will block thread
     */
    int get(Message *msg, bool block);

    /**
     * remove message by what
     */
    void remove(int what);

    /**
     * abort message queue
     */
    void setAbortRequest(bool abortRequest);

    /**
     * clear queue
     */
    void clear();

    void start();
};


#endif
