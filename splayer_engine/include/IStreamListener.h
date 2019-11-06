#ifndef ENGINE__ISTREAM_LISTENER_H
#define ENGINE__ISTREAM_LISTENER_H

class IStreamListener {

public:
    virtual int onStartOpenStream() = 0;

    virtual int onEndOpenStream(int videoIndex, int audioIndex) = 0;
};

#endif
