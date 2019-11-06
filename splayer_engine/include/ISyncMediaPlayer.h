#ifndef ENGINE_INNER_MEDIA_PLAYER_H
#define ENGINE_INNER_MEDIA_PLAYER_H

class ISyncMediaPlayer {

public:

    virtual int syncStart() = 0;

    virtual int syncStop() = 0;

    virtual int syncPause() = 0;

    virtual int syncPlay() = 0;

    virtual int syncDestroy() = 0;

    virtual int syncCreate() = 0;

    virtual int syncSeekTo(float increment) = 0;

};


#endif
