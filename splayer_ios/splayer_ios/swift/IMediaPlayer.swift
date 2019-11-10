import Foundation

public protocol IMediaPlayer {

    var rotate: Int { get }

    var videoWidth: Int { get }

    var videoHeight: Int { get }

    var isPlaying: Bool { get }

    var currentPosition: Int { get }

    var duration: Int { get }

    var isLooping: Bool { get }

    var audioSessionId: Int { get }

    func setDisplay()

    func setSurface()

    func setDataSource(path: String)

    func setDataSource(path: String, headers: Dictionary<String, String>)

    func start()

    func stop()

    func pause()

    func play()

    func setWakeMode(mode: Int)

    func setScreenOnWilePlaying(screenOn: Bool)

    func seekTo(msec: Float)

    func release()

    func reset()

    func setVolume(leftVolume: Float, rightVolume: Float)

    func setMute(mute: Bool)

    func setRate(rate: Float)

    func setPitch(pitch: Float)

    func setOnListener(listener: IOnListener?)

}
