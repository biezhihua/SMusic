#ifndef SPLAYER_MAC_DEMO_ISTREAMLISTENER_H
#define SPLAYER_MAC_DEMO_ISTREAMLISTENER_H

class IStreamListener {

public:
    virtual void onStartOpenStream() = 0;

    virtual void onEndOpenStream(int videoIndex, int audioIndex) = 0;
};

#endif
