package com.bzh.player.example

import android.content.pm.PackageManager
import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.bzh.player.R


@Suppress("UNUSED_PARAMETER")
class MainActivity : AppCompatActivity() {


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<String>, grantResults: IntArray
    ) {
        when (requestCode) {
            1 -> {
                // If request is cancelled, the result arrays are empty.
                if (grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {

                    // permission was granted, yay! Do the
                    // contacts-related task you need to do.

                } else {

                    // permission denied, boo! Disable the
                    // functionality that depends on this permission.
                }
                return
            }
        }// other 'case' lines to check for other
        // permissions this app might request
    }

    fun create(v: View) {

    }

    fun setSource(v: View) {

    }

    fun start(v: View) {
    }

    fun play(v: View) {
    }

    fun pause(v: View) {
    }

    fun stop(v: View) {
    }

    fun destroy(v: View) {
    }

    fun left(v: View) {
    }

    fun right(v: View) {
    }

    fun center(v: View) {
    }

    override fun onResume() {
        super.onResume()
    }

    override fun onPause() {
        super.onPause()
    }

    companion object {
        private const val TAG = "MainActivity"
    }
}
