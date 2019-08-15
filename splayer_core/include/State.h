#ifndef SPLAYER_STATE_H
#define SPLAYER_STATE_H

#include "Log.h"
#include "Error.h"
#include "MessageQueue.h"

#define STATE_TAG "State"

class State {

private:

    volatile int state;

    MessageQueue *msgQueue;

    const char *getState(int state);

public:
    State();

    ~State();

    /**
    * set_data_source()  -> STATE_INITIALIZED
    *
    * reset              -> self
    * release            -> STATE_END
    */
    static const int STATE_IDLE = 0;

    /**
     * prepare_async()    -> STATE_ASYNC_PREPARING
     *
     * reset              -> STATE_IDLE
     * release            -> STATE_END
     */
    static const int STATE_INITIALIZED = 1;

    /**
     *                   ...    -> STATE_PREPARED
     *                   ...    -> STATE_ERROR
     *
     * reset              -> STATE_IDLE
     * release            -> STATE_END
     */
    static const int STATE_ASYNC_PREPARING = 2;

    /**
     * seek_to()          -> self
     * start()            -> STATE_STARTED
     *
     * reset              -> STATE_IDLE
     * release            -> STATE_END
     */
    static const int STATE_PREPARED = 3;

    /**
     * seek_to()          -> self
     * start()            -> self
     * pause()            -> STATE_PAUSED
     * stop()             -> STATE_STOPPED
     *                   ...    -> STATE_COMPLETED
     *                   ...    -> STATE_ERROR
     *
     * reset              -> STATE_IDLE
     * release            -> STATE_END
     */
    static const int STATE_STARTED = 4;

    /**
     * seek_to()          -> self
     * start()            -> STATE_STARTED
     * pause()            -> self
     * stop()             -> STATE_STOPPED
     *
     * reset              -> STATE_IDLE
     * release            -> STATE_END
     */
    static const int STATE_PAUSED = 5;

    /**
     * seek_to()          -> self
     * start()            -> STATE_STARTED (from beginning)
     * pause()            -> self
     * stop()             -> STATE_STOPPED
     *
     * reset              -> STATE_IDLE
     * release            -> STATE_END
     */
    static const int STATE_COMPLETED = 6;

    /**
     * stop()             -> self
     * prepare_async()    -> STATE_ASYNC_PREPARING
     *
     * reset              -> STATE_IDLE
     * release            -> STATE_END
     */
    static const int STATE_STOPPED = 7;

    /**
     * reset              -> STATE_IDLE
     * release            -> STATE_END
     */
    static const int STATE_ERROR = 8;

    /**
     * release            -> self
     */
    static const int STATE_END = 9;

    int changeState(int state);

    void setMsgQueue(MessageQueue *msgQueue);
};


#endif //SPLAYER_STATE_H
