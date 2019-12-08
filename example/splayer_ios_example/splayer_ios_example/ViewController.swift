//
//  ViewController.swift
//  splayer_ios_example
//
//  Created by biezhihua on 2019/12/5.
//  Copyright © 2019 biezhihua. All rights reserved.
//

import UIKit
import splayer_ios

class ViewController: UIViewController {

    @IBOutlet weak var create: UIButton!

    @IBOutlet weak var start: UIButton!

    @IBOutlet weak var play: UIButton!

    @IBOutlet weak var pause: UIButton!

    @IBOutlet weak var stop: UIButton!

    @IBOutlet weak var destroy: UIButton!

    @IBAction func onCreate() {
        Log.d("Main", "onCreate")
    }

    @IBAction func onStart() {
        Log.d("Main", "onStart")
    }

    @IBAction func onPlay() {
        Log.d("Main", "onPlay")
    }

    @IBAction func onPuase() {
        Log.d("Main", "onPuase")
    }

    @IBAction func onStop() {
        Log.d("Main", "onStop")
    }

    @IBAction func onDestroy() {
        Log.d("Main", "onDestroy")
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view.
    }

}

