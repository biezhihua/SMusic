#ifndef SPLAYER_CORE_EVENT_H
#define SPLAYER_CORE_EVENT_H

class Stream;

#include "Error.h"
#include "Stream.h"
#include "Options.h"
#include "MediaPlayer.h"
#include "Surface.h"

class Event {

protected:
    Stream *stream = nullptr;
    Options *options = nullptr;
    MediaPlayer *mediaPlayer = nullptr;
    Surface *surface = nullptr;

public:
    Event();

    ~Event();

    virtual int create();

    virtual int destroy();

    void setStream(Stream *stream);

    void setOptions(Options *options);

    void setMediaPlayer(MediaPlayer *mediaPlayer);

    void setSurface(Surface *surface);

};


#endif //SPLAYER_CORE_EVENT_H
