#ifndef ENGINE_IMEDIA_PLAYER_H
#define ENGINE_IMEDIA_PLAYER_H

class IMediaPlayer {

protected:
    virtual int create() = 0;

    virtual int start() = 0;

    virtual int play() = 0;

    virtual int pause() = 0;

    virtual int stop() = 0;

    virtual int destroy() = 0;

};


#endif //MEDIAPLAYER_H
