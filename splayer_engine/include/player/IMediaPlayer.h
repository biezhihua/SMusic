#ifndef ENGINE_IMEDIA_PLAYER_H
#define ENGINE_IMEDIA_PLAYER_H

class IMediaPlayer {

public:

    virtual int create() = 0;

    virtual int start() = 0;

    virtual int play() = 0;

    virtual int pause() = 0;

    virtual int stop() = 0;

    virtual int destroy() = 0;

    virtual bool isPlaying() = 0;

    /// 设置播放状态
    virtual int setPlaying(bool isPlaying) = 0;

};


#endif
