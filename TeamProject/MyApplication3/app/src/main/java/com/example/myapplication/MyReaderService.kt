package com.example.myapplication

import android.app.Service
import android.content.Intent
import android.os.Binder
import android.os.IBinder

class MyReaderService : Service() {

    internal var switchPressed : Boolean = false
    internal var data = 0
    internal lateinit var reader : Thread
    internal var mBinder : IBinder = MyBinder()

    inner class MyBinder : Binder() {
        fun getService() : MyReaderService{
            return this@MyReaderService
        }
//        val service : MyReaderService
//            get() = this@MyReaderService
    }
    fun popData() : Int {
        val tmp = data
        data = 0
        return tmp
    }
    fun setSwitchPressed() {
        switchPressed = false
    }
    fun getSwitchPressed() : Boolean {
        return switchPressed
    }
    override fun onCreate() {
        super.onCreate()
        reader = Thread(Reader())
        reader.start()
    }
    override fun onBind(intent: Intent): IBinder {
        switchPressed = false
        data = 0
        return mBinder
    }

    private inner class Reader : Runnable{
        override fun run() {
            while(true){
                Thread.sleep(100)
//                println("Pressed : " + switchPressed)
                if(!switchPressed) {
                    var ret = readSwitchFromJNI2(GlobalVariable.fd)
                    if(ret != -1)
                        data = ret
                    else if(data > 0 && ret == -1)
                        switchPressed = true
                }
            }
        }
    }
    external fun readSwitchFromJNI2(fd : Int) : Int
}


//package com.example.myapplication
//
//import android.app.Service
//import android.content.Intent
//import android.os.Binder
//import android.os.IBinder
//
//
///* Service for timer.
// * PuzzleActivity uses this Service and get time from binder.
// * */
//class MyService : Service() {
//
//    internal var time = 0
//        private set
//    private var gameover = false
//    private var timer: Thread? = null
//    internal var mBinder: IBinder = MyBinder()
//
//    internal inner class MyBinder : Binder() {
//        val service: MyService
//            get() = this@MyService
//    }
//
//    override fun onBind(intent: Intent): IBinder? {
//        println("onBind")
//        time = 0
//        gameover = false
//        return mBinder // binder
//    }
//
//    override fun onCreate() {
//        println("onCreate")
//        super.onCreate()
//        timer = Thread(Timer()) /* Start timer thread. */
//        timer!!.start()
//    }
//
//    override fun onDestroy() {
//        println("onDestroy")
//        super.onDestroy()
//    }
//
//    override fun onUnbind(intent: Intent): Boolean {
//        println("onUnbind")
//        gameover = true
//        return super.onUnbind(intent)
//    }
//
//    /* Timer thread for timer function. */
//    private inner class Timer : Runnable {
//
//        override fun run() {
//            while (!gameover) {
//                try {
//                    Thread.sleep(1000)
//                } catch (e: InterruptedException) {
//                    e.printStackTrace()
//                }
//
//                time++
//            }
//        }
//    }
//}