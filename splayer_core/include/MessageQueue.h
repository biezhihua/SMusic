#ifndef SPLAYER_MESSAGEQUEUE_H
#define SPLAYER_MESSAGEQUEUE_H

#include "Log.h"
#include "Error.h"
#include "Mutex.h"
#include "Message.h"
#include <list>
#include <queue>

using namespace std;

#define MESSAGE_QUEUE_TAG "MsgQueue"

class MessageQueue {
private:

    Mutex *mutex = nullptr;

    list<Message *> *queue = nullptr;

    bool abortRequest = true;

private:

    int _putMsg(Message *msg);
public:
    bool isAbortRequest() const;

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
    int setAbortRequest(bool abortRequest);

    int removeMsg(int what);

    /**
     * clear queue
     */
    int clearMsgQueue();

    int startMsgQueue();

    int notifyMsg(int what);

    int notifyMsg(int what, int arg1);

    int notifyMsg(int what, int arg1, int arg2);

};


#endif
