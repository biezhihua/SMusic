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

    bool abortRequest = true;

private:
    int _putMsg(Message *msg);

public:

    MessageQueue();

    ~MessageQueue();

    /**
     * push message to queue
     */
    int putMsg(Message *msg);

    /**
     * get first message from queue, will block thread
     */
    int getMsg(Message *msg, bool block);

    /**
     * abort message queue
     */
    void setAbortRequest(bool abortRequest);

    void removeMsg(int what);

    /**
     * clear queue
     */
    void clearMsgQueue();

    int startMsgQueue();

    void notifyMsg1(int what);

    void notifyMsg2(int what, int arg1);

    void notifyMsg3(int what, int arg1, int arg2);


};


#endif
