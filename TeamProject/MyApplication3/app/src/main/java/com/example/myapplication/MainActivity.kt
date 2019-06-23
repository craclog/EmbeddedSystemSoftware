package com.example.myapplication

import android.content.Intent
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import kotlinx.android.synthetic.main.activity_main.*
import android.content.ComponentName
import android.content.Context
import com.example.myapplication.MyReaderService
import android.os.IBinder
import android.content.ServiceConnection

object GlobalVariable {
    var fd : Int = 0
}
lateinit var ms : MyReaderService

/*
* /data/local/tmp에 있는 fpgas/device_driver/fpga_insmod.sh 실행할 것.
* */
class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)


        GlobalVariable.fd = openSwitchFromJNI()
        if(GlobalVariable.fd < 0) println("Push_switch OPEN ERROR")
        else println("Push_switch OPEN SUCCESS")

        val connection = object : ServiceConnection {
            override fun onServiceConnected(name: ComponentName, service: IBinder) {
                val mb = service as MyReaderService.MyBinder
                ms = mb.getService()
//                isServiceOn = true //
            }
            override fun onServiceDisconnected(name: ComponentName) {
//                isServiceOn = false
            }
        }
        val readerIntent : Intent = Intent(this, MyReaderService::class.java)
        bindService(readerIntent, connection, Context.BIND_AUTO_CREATE)
        ledOn_btn.setOnClickListener {
            val ret = readSwitchFromJNI(GlobalVariable.fd)
            println("Return value : %d".format(ret))
        }
        ledOff_btn.setOnClickListener {
            val pressed = ms.getSwitchPressed()
            if(pressed) {
                println("Read Data : " + ms.popData())
                ms.setSwitchPressed()
            }
            else println("No Data")
        }
        close_btn.setOnClickListener{
            closeFromJNI(GlobalVariable.fd);
            val intent = Intent(this, OmokActivity::class.java)
            println("Sending : " + id_editText.text.toString())
            intent.putExtra("id", id_editText.text.toString())
            startActivity(intent)
        }
        val reader = Thread(ReadDataThread())
        reader.start()
    }
    private inner class ReadDataThread : Runnable{
        override fun run() {
            Thread.sleep(1000)
            while(true){
                Thread.sleep(100)
                val pressed = ms.getSwitchPressed()
                if(pressed) {
                    println("Read Data : " + ms.popData())
                    ms.setSwitchPressed()
                }
            }
        }
    }
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String
    external fun openSwitchFromJNI() : Int
    external fun closeFromJNI(fd : Int) : Void
    external fun readSwitchFromJNI(fd : Int) : Int
    companion object {

        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }
}
