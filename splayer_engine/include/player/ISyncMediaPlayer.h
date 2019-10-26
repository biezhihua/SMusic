#ifndef ENGINE_INNER_MEDIA_PLAYER_H
#define ENGINE_INNER_MEDIA_PLAYER_H

class ISyncMediaPlayer {

public:

    virtual int syncStop() = 0;

    virtual int syncPause() = 0;

    virtual int syncPlay() = 0;

};


#endif
