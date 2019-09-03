#ifndef AVMESSAGEQUEUE_H
#define AVMESSAGEQUEUE_H

#include <common/Mutex.h>
#include <common/Condition.h>
#include <cstring>
#include <assert.h>

extern "C" {
#include <libavutil/mem.h>
};

#include "PlayerMessage.h"

typedef struct Message {
    int what;
    int arg1;
    int arg2;
    void *obj;

    void (*free)(void *obj);

    struct Message *next;
} AVMessage;

inline static void message_init(Message *msg) {
    memset(msg, 0, sizeof(Message));
}

inline static void message_free(void *obj) {
    av_free(obj);
}

inline static void message_free_resouce(Message *msg) {
    if (!msg || !msg->obj) {
        return;
    }
    assert(msg->free);
    msg->free(msg->obj);
    msg->obj = nullptr;
}

class MessageQueue {
public:
    MessageQueue();

    virtual ~MessageQueue();

    void start();

    void stop();

    void flush();

    void release();

    void notifyMessage(int what);

    void notifyMessage(int what, int arg1);

    void notifyMsg(int what, int arg1, int arg2);

    void notifyMessage(int what, int arg1, int arg2, void *obj, int len);

    int getMessage(Message *msg);

    int getMessage(Message *msg, int block);

    void removeMessage(int what);

private:
    int putMessage(Message *msg);

private:
    Mutex mutex;
    Condition condition;
    Message *firstMsg, *lastMsg;
    bool abortRequest;
    int size;
};

#endif //AVMESSAGEQUEUE_H
