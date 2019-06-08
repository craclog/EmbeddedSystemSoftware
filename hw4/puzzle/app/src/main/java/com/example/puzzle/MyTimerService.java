package com.example.puzzle;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;

public class MyTimerService extends Service {

    private int time = 0;
    private boolean gameover = false;
    private Thread timer;

    public MyTimerService() {
    }

    IMyTimerInterface.Stub binder = new IMyTimerInterface.Stub() {

        @Override
        public int getTime() {
            return time;
        }
    };

    @Override
    public IBinder onBind(Intent intent) {
        System.out.println("onBind");
        time = 0;
        gameover = false;
        return binder;
    }

    @Override
    public void onCreate(){
        System.out.println("onCreate");
        super.onCreate();
        timer = new Thread(new Timer());
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

    private class Timer implements Runnable {

        @Override
        public void run() {
            System.out.println("Thread Run!");
            while(!gameover){
                time++;
                System.out.println("Time : " + time);
                try{
                    Thread.sleep(1000);
                }catch (InterruptedException e){
                    e.printStackTrace();
                }
            }
        }
    }
}
