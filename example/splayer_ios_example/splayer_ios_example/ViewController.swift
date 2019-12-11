import UIKit
import splayer_ios

class ViewController: UIViewController {
    
    @IBOutlet weak var create: UIButton!
    
    @IBOutlet weak var start: UIButton!
    
    @IBOutlet weak var play: UIButton!
    
    @IBOutlet weak var pause: UIButton!
    
    @IBOutlet weak var stop: UIButton!
    
    @IBOutlet weak var destroy: UIButton!
    
    @IBOutlet weak var renderView: UIView!
    
    var mediaPlayer: MediaPlayer?
    
    var mediaPlayer2: MediaPlayer?
    
    @IBAction func onCreate() {
        Log.d("Main", "onCreate")
        mediaPlayer = MediaPlayer()
        mediaPlayer?.setSurface(renderView)
        
        mediaPlayer2 = MediaPlayer()
        mediaPlayer2?.setSurface(renderView)
        
        
    }
    
    @IBAction func onStart() {
        Log.d("Main", "onStart \(mediaPlayer)")
        Log.d("Main", "onStart \(mediaPlayer2)")
        mediaPlayer?.start()
     mediaPlayer2?.start()
    }
    
    @IBAction func onPlay() {
        Log.d("Main", "onPlay")
        mediaPlayer?.play()
    }
    
    @IBAction func onPause() {
        Log.d("Main", "onPause")
        mediaPlayer?.pause()
    }
    
    @IBAction func onStop() {
        Log.d("Main", "onStop")
        mediaPlayer?.stop()
    }
    
    @IBAction func onDestroy() {
        Log.d("Main", "onDestroy")
        mediaPlayer = nil
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view.
    }
    
}

