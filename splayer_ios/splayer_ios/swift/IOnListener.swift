import Foundation

public protocol IOnListener {
    func onStarted(mp: IMediaPlayer)

    func onCompletion(mp: IMediaPlayer)

    func onInfo(mp: IMediaPlayer, what: Int, extra: Int) -> Bool

    func onError(mp: IMediaPlayer, what: Int, extra: Int) -> Bool

    func onTimedText(mp: IMediaPlayer, text: ITimedText?)

    func onVideoSizeChanged(mediaPlayer: IMediaPlayer, width: Int, height: Int)

    func onSeekComplete(mp: IMediaPlayer)

    func onBufferingUpdate(mp: IMediaPlayer, percent: Int)

    func onCurrentPosition(current: Int, duration: Int)
}