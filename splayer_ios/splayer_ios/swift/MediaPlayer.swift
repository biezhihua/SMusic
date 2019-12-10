import Foundation
import splayer_ios_birdge
import UIKit

public class MediaPlayer: IMediaPlayer {
    private let TAG: String = "[MP][LIB][MediaPlayer]"

    private var nativeContext: Int64 = 0

    private var onListener: IOnListener? = nil

    public init() {
        IOS_DEBUG = true

        if (IOS_DEBUG) {
            Log.d(TAG, "init")
        }

        //        SwiftFunc = swiftFuncImpl
        //        CFuncTest()
        _create(&nativeContext)
        audioSessionId = 0
    }

    // 这里是对SwiftFunc的实现
    private func swiftFuncImpl() {
        print("This is a Swift function!");
    }

    deinit {
        if (IOS_DEBUG) {
            Log.d(TAG, "deinit")
        }
        _destroy(&nativeContext)
    }

    public var rotate: Int {
        get {
            _getRotate(&nativeContext)
        }
    }

    public var videoWidth: Int {
        get {
            _getVideoWidth(&nativeContext)
        }
    }

    public var videoHeight: Int {
        get {
            _getVideoHeight(&nativeContext)
        }
    }

    public var isPlaying: Bool {
        get {
            _isPlaying(&nativeContext)
        }
    }

    public var currentPosition: Int {
        get {
            _getCurrentPosition(&nativeContext)
        }
    }

    public var duration: Int {
        get {
            _getDuration(&nativeContext)
        }
    }

    public var isLooping: Bool {
        get {
            _isLooping(&nativeContext)
        }
    }

    public var audioSessionId: Int

    public func setDisplay() {

    }

    public func setSurface(_ surface: UIView?) {
        if surface != nil {
            _setSurface(&nativeContext)
        }
    }

    public func setDataSource(path: String) {
        _setDataSource(&nativeContext, path)
    }

    public func setDataSource(path: String, headers: Dictionary<String, String>) {
        _setDataSourceAndHeaders(&nativeContext, path, nil, nil)
    }


    public func start() {

        if (IOS_DEBUG) {
            Log.d(TAG, "start")
        }

        stayAwake(awake: true)
        _start(&nativeContext)
    }

    private func stayAwake(awake: Bool) {
    }

    public func stop() {
        if (IOS_DEBUG) {
            Log.d(TAG, "stop")
        }
        stayAwake(awake: false)
        _stop(&nativeContext)
    }

    public func pause() {
        if (IOS_DEBUG) {
            Log.d(TAG, "pause")
        }
        stayAwake(awake: false)
        _pause(&nativeContext)
    }

    public func play() {
        if (IOS_DEBUG) {
            Log.d(TAG, "play")
        }
        stayAwake(awake: true)
        _play(&nativeContext)
    }

    public func setWakeMode(mode: Int) {
    }

    public func setScreenOnWilePlaying(screenOn: Bool) {
    }

    public func seekTo(msec: Float) {
        _seekTo(&nativeContext, msec)
    }

    public func release() {
        if (IOS_DEBUG) {
            Log.d(TAG, "release")
        }
        stayAwake(awake: false)
        onListener = nil
        _destroy(&nativeContext)
    }

    public func reset() {
        if (IOS_DEBUG) {
            Log.d(TAG, "reset")
        }
        stayAwake(awake: false)
        _reset(&nativeContext)
    }

    public func setVolume(leftVolume: Float, rightVolume: Float) {
        _setVolume(&nativeContext, leftVolume, rightVolume)
    }

    public func setMute(mute: Bool) {
        _setMute(&nativeContext, mute)
    }

    public func setRate(rate: Float) {
        _setRate(&nativeContext, rate)
    }

    public func setPitch(pitch: Float) {
        _setPitch(&nativeContext, pitch)
    }

    public func setOnListener(listener: IOnListener?) {
        onListener = listener
    }

    public func setOption(category: Int, type: String, option: String) {
        _setOptionS(&nativeContext, category, type, option)
    }

    public func setOption(category: Int, type: String, option: Int) {
        _setOptionL(&nativeContext, category, type, option)
    }

    public func setDebug(debug: Bool) {
        if (IOS_DEBUG) {
            Log.d(TAG, "setDebug : debug = \(debug)")
        }
        IOS_DEBUG = debug

    }

}
