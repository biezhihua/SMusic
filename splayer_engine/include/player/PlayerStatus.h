#ifndef ENGINE_PLAYERSTATUS_H
#define ENGINE_PLAYERSTATUS_H

/**
 * MediaPlayer Status 播放器状态
 *
 * IDLED            -> 已闲置态
 * CREATED          -> 已创建态
 * STARTED          -> 已启动态
 * PLAYING          -> 已播放态
 * PAUSED           -> 已暂停态
 * STOPED           -> 已停止态
 * DESTROYED        -> 已销毁态
 * ERRORED          -> 已错误态
 */
enum PlayerStatus {
    /**
     * new MediaPlayer()        => self
     * destroy()                => self
     * reset()                  => self
     * create()                 => CREATED
     */
            IDLED = 1,
    /**
     * create()                 => self
     * start()                  => STARTED
     * destroy()                => DESTROYED -> IDLE
     * reset()                  => DESTROYED -> IDLE
     */
            CREATED = 2,
    /**
     * start()                  => self
     * play()                   => self
     * pause()                  => PAUSED
     * stop()                   => STOPED
     * destroy()                => STOPED -> DESTROYED -> IDLE
     * reset()                  => STOPED -> DESTROYED -> IDLE
     */
            STARTED = 3,
    /**
     * STARTED & read a frame   => self
     * seek()                   => self
     * pause()                  => PAUSED
     * stop()                   => STOPED
     * destroy()                => STOPED -> DESTROYED -> IDLE
     * reset()                  => STOPED -> DESTROYED -> IDLE
     */
            PLAYING = 4,
    /**
     * seek()                   => self
     * paused()                 => self
     * play()                   => STARTED
     * stop()                   => STOPED
     * destroy()                => STOPED -> DESTROYED -> IDLE
     * reset()                  => STOPED -> DESTROYED -> IDLE
     */
            PAUSED = 5,
    /**
     * stop()                   => self
     * start()                  => STARTED
     * destroy()                => DESTROYED -> IDLE
     * reset()                  => DESTROYED -> IDLE
     */
            STOPPED = 6,
    /**
     * destroy()                => self -> IDLE
     * reset()                  => self -> IDLE
     */
            DESTROYED = 7,
    /**
     *  any error               => ERRORED
     *  reset()                 => DESTROYED -> IDLE
     *  destroy()               => any -> DESTROY -> IDLE
     */
            ERRORED = 8
};

#endif
