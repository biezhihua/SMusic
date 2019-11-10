import Foundation
import splayer_ios_birdge

public class MediaPlayer: IMediaPlayer {

    public init() {
        rotate = 0
        videoWidth = 0
        videoHeight = 0
        isPlaying = false
        currentPosition = 0
        duration = 0
        isLooping = false
        audioSessionId = 0
    }

    deinit {

    }

    public var rotate: Int

    public var videoWidth: Int

    public var videoHeight: Int

    public var isPlaying: Bool

    public var currentPosition: Int

    public var duration: Int

    public var isLooping: Bool

    public var audioSessionId: Int

    public func setDisplay() {

    }

    public func setSurface() {
    }

    public func setDataSource(path: String) {
    }

    public func setDataSource(path: String, headers: Dictionary<String, String>) {
    }

    public func start() {
    }

    public func stop() {
    }

    public func pause() {
    }

    public func play() {
    }

    public func setWakeMode(mode: Int) {
    }

    public func setScreenOnWilePlaying(screenOn: Bool) {
    }

    public func seekTo(msec: Float) {
    }

    public func release() {
    }

    public func reset() {
    }

    public func setVolume(leftVolume: Float, rightVolume: Float) {
    }

    public func setMute(mute: Bool) {
    }

    public func setRate(rate: Float) {
    }

    public func setPitch(pitch: Float) {
    }

    public func setOnListener(listener: IOnListener) {
    }


}
