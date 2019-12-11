import Foundation
import splayer_ios_birdge
import UIKit

public class MediaPlayer: IMediaPlayer {
    private let TAG: String = "[MP][LIB][MediaPlayer]"

    public class SMPReference {
        var mediaPlayer: MediaPlayer? = nil
    }

    private var nmpReference: Int64 = 0

    private var smpReference: SMPReference = SMPReference()

    private var onListener: IOnListener? = nil

    public init() {
        IOS_DEBUG = true
        if (IOS_DEBUG) {
            Log.d(TAG, "init")
        }
        _postFromNative = { (reference: UnsafeMutableRawPointer?, msg: Int32, ext1: Int32, ext2: Int32) -> Void in
            if (reference != nil) {
                let currentReference = reference!.load(as: SMPReference.self)
                currentReference.mediaPlayer?.runOnUI(msg, ext1, ext2)
            }
        }
                   
        smpReference.mediaPlayer = self
        _create(&nmpReference, UnsafeMutableRawPointer(&smpReference))
    }


    deinit {
        if (IOS_DEBUG) {
            Log.d(TAG, "deinit")
        }
        _destroy(&nmpReference)
    }

    public func runOnUI(_ msg: Int32, _ ext1: Int32, _  ext2: Int32) {
        DispatchQueue.main.async() {
            if (IOS_DEBUG) {

                Log.d(self.TAG, "runMainThread self = \(Unmanaged.passUnretained(self).toOpaque()) msg = \(msg) ext1 = \(ext1) ext2 = \(ext2)")
            }
        }
    }

    public var rotate: Int {
        get {
            _getRotate(&nmpReference)
        }
    }

    public var videoWidth: Int {
        get {
            _getVideoWidth(&nmpReference)
        }
    }

    public var videoHeight: Int {
        get {
            _getVideoHeight(&nmpReference)
        }
    }

    public var isPlaying: Bool {
        get {
            _isPlaying(&nmpReference)
        }
    }

    public var currentPosition: Int {
        get {
            _getCurrentPosition(&nmpReference)
        }
    }

    public var duration: Int {
        get {
            _getDuration(&nmpReference)
        }
    }

    public var isLooping: Bool {
        get {
            _isLooping(&nmpReference)
        }
    }

    public var audioSessionId: Int = 0

    public func setDisplay() {

    }

    public func setSurface(_ surface: UIView?) {
        if surface != nil {
            _setSurface(&nmpReference)
        }
    }

    public func setDataSource(path: String) {
        _setDataSource(&nmpReference, path)
    }

    public func setDataSource(path: String, headers: Dictionary<String, String>) {
        _setDataSourceAndHeaders(&nmpReference, path, nil, nil)
    }


    public func start() {

        if (IOS_DEBUG) {
            Log.d(TAG, "start")
        }

        stayAwake(awake: true)
        _start(&nmpReference)
    }

    private func stayAwake(awake: Bool) {
    }

    public func stop() {
        if (IOS_DEBUG) {
            Log.d(TAG, "stop")
        }
        stayAwake(awake: false)
        _stop(&nmpReference)
    }

    public func pause() {
        if (IOS_DEBUG) {
            Log.d(TAG, "pause")
        }
        stayAwake(awake: false)
        _pause(&nmpReference)
    }

    public func play() {
        if (IOS_DEBUG) {
            Log.d(TAG, "play")
        }
        stayAwake(awake: true)
        _play(&nmpReference)
    }

    public func setWakeMode(mode: Int) {
    }

    public func setScreenOnWilePlaying(screenOn: Bool) {
    }

    public func seekTo(msec: Float) {
        _seekTo(&nmpReference, msec)
    }

    public func release() {
        if (IOS_DEBUG) {
            Log.d(TAG, "release")
        }
        stayAwake(awake: false)
        onListener = nil
        _destroy(&nmpReference)
    }

    public func reset() {
        if (IOS_DEBUG) {
            Log.d(TAG, "reset")
        }
        stayAwake(awake: false)
        _reset(&nmpReference)
    }

    public func setVolume(leftVolume: Float, rightVolume: Float) {
        _setVolume(&nmpReference, leftVolume, rightVolume)
    }

    public func setMute(mute: Bool) {
        _setMute(&nmpReference, mute)
    }

    public func setRate(rate: Float) {
        _setRate(&nmpReference, rate)
    }

    public func setPitch(pitch: Float) {
        _setPitch(&nmpReference, pitch)
    }

    public func setOnListener(listener: IOnListener?) {
        onListener = listener
    }

    public func setOption(category: Int, type: String, option: String) {
        _setOptionS(&nmpReference, category, type, option)
    }

    public func setOption(category: Int, type: String, option: Int) {
        _setOptionL(&nmpReference, category, type, option)
    }

    public func setDebug(debug: Bool) {
        if (IOS_DEBUG) {
            Log.d(TAG, "setDebug : debug = \(debug)")
        }
        IOS_DEBUG = debug
    }

}
