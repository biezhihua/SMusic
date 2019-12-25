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
    
    
    @IBAction func onCreate() {
        Log.d("Main", "onCreate")
        mediaPlayer = MediaPlayer()
        mediaPlayer?.setSurface(renderView)
        
        let videoPath = getTestVideoPath()
        Log.d("Main", "videoPath=\(videoPath)")
        mediaPlayer?.setDataSource(path:videoPath)
        
    }
    
    func getTestVideoPath() -> String {
        let fileManager = FileManager.default
        let documentsURL = fileManager.urls(for: .documentDirectory, in: .userDomainMask)[0]
        do {
            let fileURLs = try fileManager.contentsOfDirectory(at: documentsURL, includingPropertiesForKeys: nil)
            
            
            fileURLs.forEach { (url) in
                Log.d("Main", url.absoluteString)
            }
            
            
            return fileURLs.last!.absoluteString
        
            
        } catch {
            print("Error while enumerating files \(documentsURL.path): \(error.localizedDescription)")
            return ""
        }
        return ""
    }
    
    @IBAction func onStart() {
        Log.d("Main", "onStart \(mediaPlayer)")
        
        mediaPlayer?.start()
        
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
        let DocumentDirectory = NSURL(fileURLWithPath: NSSearchPathForDirectoriesInDomains(.documentDirectory, .userDomainMask, true)[0])
        let DirPath = DocumentDirectory.appendingPathComponent("SPLAYER_DEMO")
        
    }
    
}



