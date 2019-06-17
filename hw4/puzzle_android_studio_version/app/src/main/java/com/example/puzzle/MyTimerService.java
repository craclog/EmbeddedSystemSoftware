package com.example.puzzle;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;


/* Service for timer.
 * PuzzleActivity uses this Service and get time from binder.
 * */
public class MyTimerService extends Service {

    private int time = 0;
    private boolean gameover = false;
    private Thread timer;

    public MyTimerService() {
    }
    IBinder mBinder = new MyBinder();

    class MyBinder extends Binder {
        MyTimerService getService(){
            return MyTimerService.this;
        }
    }
    int getTime(){
        return time;
    }
    @Override
    public IBinder onBind(Intent intent) {
        System.out.println("onBind");
        time = 0;
        gameover = false;
        return mBinder; // binder
    }

    @Override
    public void onCreate(){
        System.out.println("onCreate");
        super.onCreate();
        timer = new Thread(new Timer()); /* Start timer thread. */
        timer.start();
    }

    @Override
    public void onDestroy(){
        System.out.println("onDestroy");
        super.onDestroy();
    }

    @Override
    public boolean onUnbind(Intent intent) {
        System.out.println("onUnbind");
        gameover = true;
        return super.onUnbind(intent);
    }
    /* Timer thread for timer function. */
    private class Timer implements Runnable {

        @Override
        public void run() {
            while(!gameover){
                try{
                    Thread.sleep(1000);
                }catch (InterruptedException e){
                    e.printStackTrace();
                }
                time++;
            }
        }
    }
}
