#ifndef SPLAYER_MESSAGEQUEUE_H
#define SPLAYER_MESSAGEQUEUE_H

#include "Log.h"
#include "Common.h"
#include "Mutex.h"
#include "Message.h"
#include <list>
#include <queue>


using namespace std;

class MessageQueue {
private:

    Mutex *pMutex = nullptr;

    list<Message *> *pQueue = nullptr;

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

    int start();

    void notifyMsg1(int what);

    void notifyMsg2(int what, int arg1);

    void notifyMsg3(int what, int arg1, int arg2);

    void removeMsg(int what);
};


#endif
