#ifndef SPLAYER_MAC_DEMO_ISTREAMLISTENER_H
#define SPLAYER_MAC_DEMO_ISTREAMLISTENER_H

class IStreamListener {

public:
    virtual int onStartOpenStream() = 0;

    virtual int onEndOpenStream(int videoIndex, int audioIndex) = 0;
};

#endif
