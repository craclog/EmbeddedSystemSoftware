package com.example.myapplication

import android.content.Intent
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import kotlinx.android.synthetic.main.activity_main.*
import android.content.ComponentName
import android.content.Context
import android.os.IBinder
import android.content.ServiceConnection

object GlobalVariable {
    var fd : Int = 0
}
lateinit var ms : MyReaderService

/*
 * /data/local/tmp에 있는 fpgas/device_driver/fpga_insmod.sh 실행할 것.
 */
class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)



//        ledOn_btn.setOnClickListener {
//            val ret = readSwitchFromJNI(GlobalVariable.fd)
//            println("Return value : %d".format(ret))
//        }
//        ledOff_btn.setOnClickListener {
//            val pressed = ms.getSwitchPressed()
//            if(pressed) {
//                println("Read Data : " + ms.popData())
//                ms.setSwitchPressed()
//            }
//            else println("No Data")
//        }
        start_btn.setOnClickListener{
            val intent = Intent(this, OmokActivity::class.java)
            intent.putExtra("id", id_editText.text.toString())
            startActivity(intent)
        }

    }
    companion object {
        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }
}
