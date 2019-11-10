import Foundation
import splayer_ios_birdge

public class MediaPlayer: IMediaPlayer {

    private let TAG: String = "[MP][LIB][MediaPlayer]"

    public var IOS_DEBUG: Bool = true

    private var onListener: IOnListener? = nil

    public init() {
        if (IOS_DEBUG) {
            Log.d(TAG, "init")
        }
        _native_init()
        audioSessionId = 0
    }

    deinit {
        if (IOS_DEBUG) {
            Log.d(TAG, "deinit")
        }
        _native_deinit()
    }

    public var rotate: Int {
        get {
            _getRotate()
        }
    }

    public var videoWidth: Int {
        get {
            _getVideoWidth()
        }
    }

    public var videoHeight: Int {
        get {
            _getVideoHeight()
        }
    }

    public var isPlaying: Bool {
        get {
            _isPlaying()
        }
    }

    public var currentPosition: Int {
        get {
            _getCurrentPosition()
        }
    }

    public var duration: Int {
        get {
            _getDuration()
        }
    }

    public var isLooping: Bool {
        get {
            _isLooping()
        }
    }

    public var audioSessionId: Int

    public func setDisplay() {

    }

    public func setSurface() {
    }

    public func setDataSource(path: String) {
        _setDataSource(path)
    }

    public func setDataSource(path: String, headers: Dictionary<String, String>) {
        _setDataSourceAndHeaders(path, nil, nil)
    }

    public func start() {
        if (IOS_DEBUG) {
            Log.d(TAG, "start")
        }
        stayAwake(awake: true)
        _start()
    }

    private func stayAwake(awake: Bool) {
    }

    public func stop() {
        if (IOS_DEBUG) {
            Log.d(TAG, "stop")
        }
        stayAwake(awake: false)
        _stop()
    }

    public func pause() {
        if (IOS_DEBUG) {
            Log.d(TAG, "pause")
        }
        stayAwake(awake: false)
        _pause()
    }

    public func play() {
        if (IOS_DEBUG) {
            Log.d(TAG, "play")
        }
        stayAwake(awake: true)
        _play()
    }

    public func setWakeMode(mode: Int) {
    }

    public func setScreenOnWilePlaying(screenOn: Bool) {
    }

    public func seekTo(msec: Float) {
        _seekTo(msec)
    }

    public func release() {
        if (IOS_DEBUG) {
            Log.d(TAG, "release")
        }
        stayAwake(awake: false)
        onListener = nil
        _release()
    }

    public func reset() {
        if (IOS_DEBUG) {
            Log.d(TAG, "reset")
        }
        stayAwake(awake: false)
        _reset()
    }

    public func setVolume(leftVolume: Float, rightVolume: Float) {
        _setVolume(leftVolume, rightVolume)
    }

    public func setMute(mute: Bool) {
        _setMute(mute)
    }

    public func setRate(rate: Float) {
        _setRate(rate)
    }

    public func setPitch(pitch: Float) {
        _setPitch(pitch)
    }

    public func setOnListener(listener: IOnListener?) {
        onListener = listener
    }

    public func setOption(category: Int, type: String, option: String) {
        _setOptionS(category, type, option)
    }

    public func setOption(category: Int, type: String, option: Int) {
        _setOptionL(category, type, option)
    }

    public func setDebug(debug: Bool) {
        if (IOS_DEBUG) {
            Log.d(TAG, "setDebug : debug = \(debug)")
        }
        IOS_DEBUG = debug;
    }

}
