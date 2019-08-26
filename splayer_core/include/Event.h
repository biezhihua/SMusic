#ifndef SPLAYER_CORE_EVENT_H
#define SPLAYER_CORE_EVENT_H

class Stream;

#include "Error.h"
#include "Stream.h"
#include "Options.h"
#include "MediaPlayer.h"
#include "Surface.h"

/**
 * 用于处理输入事件
 */
class Event {

protected:
    Stream *stream = nullptr;
    Options *options = nullptr;
    MediaPlayer *mediaPlayer = nullptr;
    Surface *surface = nullptr;
    MessageQueue *msgQueue = nullptr;

public:
    Event();

    virtual ~Event();

    virtual int create();

    virtual int destroy();

    virtual int eventLoop();

    void setStream(Stream *stream);

    void setOptions(Options *options);

    void setMediaPlayer(MediaPlayer *mediaPlayer);

    void setSurface(Surface *surface);

    void setMsgQueue(MessageQueue *msgQueue);



};


#endif //SPLAYER_CORE_EVENT_H
