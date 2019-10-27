#ifndef ENGINE_MESSAGE_QUEUE_H
#define ENGINE_MESSAGE_QUEUE_H

#include <list>
#include <queue>
#include <common/Mutex.h>
#include <common/Condition.h>
#include <common/Log.h>
#include <message/Msg.h>
#include "IMessageListener.h"

using namespace std;

class MessageQueue {

    const char *const TAG = "[MP][NATIVE][MsgCenter]";

private:

    Mutex mutex;

    Condition condition;

    list<Msg *> *queue = nullptr;

private:

    int _putMsg(Msg *msg);

public:

    MessageQueue();

    ~MessageQueue();

    int putMsg(Msg *msg);

    int getMsg(Msg *msg, bool block);

    int removeMsg(int what);

    int clearMsgQueue(IMessageListener *pListener);

    int startMsgQueue();

    int notifyMsg(int what);

    int notifyMsg(int what, int arg1);

    int notifyMsg(int what, float arg1);

    int notifyMsg(int what, int arg1, int arg2);

};


#endif
