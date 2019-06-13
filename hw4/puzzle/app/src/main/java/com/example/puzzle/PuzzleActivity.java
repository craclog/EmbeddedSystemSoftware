package com.example.puzzle;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Color;
import android.os.Handler;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;
import java.util.Random;
import static java.lang.Math.abs;

public class PuzzleActivity extends AppCompatActivity {

    final int PADDING  = 16;
    EditText data;
    TextView clock;
    String[] splitted;
    Button make_btn;
    LinearLayout main_layout;
    Context context;
    Button[] btns;
    int[] btns_info = new int[25];

    private boolean gameover = false;
    private int row, col;
    private int blank_row, blank_col, blank_idx;
    int btn_width, btn_height;
    MyTimerService ms;
    boolean isServiceOn = false;

    /* Make connection and get binder */
    private ServiceConnection connection  = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            /* Get binder from service. Now you can use getTime(). */
            MyTimerService.MyBinder mb = (MyTimerService.MyBinder) service;
            ms = mb.getService();
            isServiceOn = true; //
        }
        @Override
        public void onServiceDisconnected(ComponentName name) {
            isServiceOn = false;
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_puzzle);
        /* Find Buttons, plainText, LinearLayout, textView */
        make_btn = findViewById(R.id.make_btn);
        data = findViewById(R.id.input_plainText);
        main_layout = findViewById(R.id.btn_main_LinearLayout);
        clock = findViewById(R.id.time_textView);
        context = this;

        /* onClickListener for make_btn. ("Make Buttons") */
        View.OnClickListener listener = new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                /* Clear all views(buttons) */
                clear_layout();
                try{
                    /* Parse input data */
                    splitted = data.getText().toString().trim().split("\\s+");
                    if(splitted.length > 2) throw new Exception();
                    row = Integer.parseInt(splitted[0]);
                    col = Integer.parseInt(splitted[1]);
                    if(row == 0 || col == 0 || row > 5 || col > 5 || (col == 1 && row == 1)) throw new Exception();

                    /* Calculate buttons' size */
                    btn_width = calc_btn_width();
                    btn_height = calc_btn_height();

                    do{
                        /* Generate random buttons' order.
                         * Repeat this until initial state is not solved.*/
                        generate_btn_order();
                    }while(is_end_game());
                    generate_btns(); /* Make buttons dynamically */

                    /* Start Timer */
                    Intent intent_timer = new Intent(PuzzleActivity.this, MyTimerService.class);
                    /* if already bind once, unbind to reset service's time */
                    if(isServiceOn) unbindService(connection);
                    bindService(intent_timer, connection, BIND_AUTO_CREATE);
                    gameover = false;
                    new Thread(new GetTimeThread()).start();

                } catch (Exception e){
                    /* If splitted data's length > 2, or row == 0, or col == 0 */
                    Toast.makeText(getApplicationContext(), "Invalid input data [row(1~5) col(1~5)]", Toast.LENGTH_LONG).show();
                    if(isServiceOn) {
                        unbindService(connection); /* Disconnect binder */
                        isServiceOn = false;
                    }
                    gameover = true; // To finish GetTimeThread
                    clock.setText("00:00"); // Init clock TextView
                    System.out.println("Make_btn listener Exception occurred! : " + e);
                }
            }
        };
        make_btn.setOnClickListener(listener);
    }
    /* Clear all Buttons */
    public void clear_layout(){
        main_layout.removeAllViews();
    }
    /* Calculate buttons' width */
    public int calc_btn_width(){
        int w = main_layout.getWidth();
        return (w - (col - 1)*PADDING) / col;
    }
    /* Calculate buttons' height */
    public int calc_btn_height(){
        int h = main_layout.getHeight();
        return (h - (row - 1)*PADDING) / row;
    }
    /* Set buttons' text random */
    public void generate_btn_order(){
        int[][] a = new int[5][5];
        int[][] ndir = {{0,1}, {1,0}, {-1,0},{0,-1}};
        int cnt = 1, dir;
        int cur_r, cur_c, nr, nc;

        Random rand = new Random();
        for(int i=0; i<row; i++) /* First, start with solved state. */
            for(int j=0; j<col; j++)
                a[i][j] = cnt++;
        a[row-1][col-1] = 0; /* Last button is 0. */
        cur_r = row - 1; cur_c = col - 1; /* Set 0's position. */
        for(int i=0; i<1000; i++){
            dir = rand.nextInt(Integer.SIZE - 1) % 4;
            nr = cur_r + ndir[dir][0];
            nc = cur_c + ndir[dir][1];
            if(nr < 0 || nr >= row || nc < 0 || nc >= col) continue; /* if It is Out of boundary, continue */
            /* Swap two buttons and modify cur 0's position */
            a[cur_r][cur_c] = a[nr][nc];
            a[nr][nc] = 0;
            cur_r = nr; cur_c = nc;
        }
        /* Fill btns_info with a */
        for(int i=0; i<row; i++)
            for(int j=0; j<col; j++)
                btns_info[i*col + j] = a[i][j];
    }
    /* OnClickListener for Buttons */
    View.OnClickListener btn_listener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {

            int btn_idx = v.getId();
            int num_on_btn = btns_info[btn_idx];
            /* Check if it can move */
            if(is_valid_move(v, btn_idx)){
                /* Swap two buttons */
                btns_info[blank_idx] = num_on_btn;
                btns_info[btn_idx] = 0;

                /* Erase all buttons and Redraw them. */
                clear_layout();
                generate_btns();
                /* Check game end. If game over, Go back to MainActivity */
                if(is_end_game()){
                    unbindService(connection);
                    Toast.makeText(getApplicationContext(), "Congratulations, Clear!!!", Toast.LENGTH_LONG).show();
                    Intent intent = new Intent(PuzzleActivity.this, MainActivity.class);
                    startActivity(intent);
                }

            }

        }
    };
    /* Function for generating buttons */
    public void generate_btns(){

        /* Make buttons dynamically */
        btns = new Button[row*col];
        for(int r=0; r<row; r++){
            /* Make new Horizontal LinearLayout
             * and add this to main_layout */
            LinearLayout new_layout = new LinearLayout(context);
            new_layout.setOrientation(LinearLayout.HORIZONTAL);
            main_layout.addView(new_layout);
            for(int c=0; c<col; c++){
                int cur_idx = r*col + c;
                /* Make new button and add this to current row's LinearLayout */
                btns[cur_idx] = new Button(context);
                btns[cur_idx].setText(Integer.toString(btns_info[cur_idx]));
                btns[cur_idx].setId(cur_idx);
                btns[cur_idx].setWidth(btn_width);
                btns[cur_idx].setHeight(btn_height);
                new_layout.addView(btns[cur_idx]);
                /* If Button for 0, Set backgroundColor Black.
                 * Save Button 0's position and ID. */
                if(btns_info[cur_idx] == 0) {
                    btns[cur_idx].setBackgroundColor(Color.BLACK);
                    blank_idx = cur_idx;
                    blank_row = r;
                    blank_col = c;
                }
                btns[cur_idx].setOnClickListener(btn_listener);
            }
        }

    }
    /* Check whether game ends.
     * Return true if game solved. */
    public boolean is_end_game(){
        int num_of_btn = row * col - 1;
        for(int i=0; i<num_of_btn; i++){
            if(btns_info[i] != i + 1) return gameover = false   ;
        }
        return gameover = true;
    }
    /* From Button index, return row information. */
    public int id2row(int id){
        return id / col;
    }
    /* From Button index, return col information. */
    public int id2col(int id){
        return id % col;
    }
    /* Check whether it is valid movement. */
    public boolean is_valid_move(View v, int btn_idx){

        int btn_row = id2row(btn_idx);
        int btn_col = id2col(btn_idx);

        if(btn_idx == blank_idx) return false; /* If pressed button is 0. */
        else if(btn_row == blank_row){ /* If pressed button is beside button 0. */
            if( abs(btn_col - blank_col) <= 1) return true;
        }
        else if(btn_col == blank_col){ /* If pressed button is beside button 0. */
            if( abs(btn_row - blank_row) <= 1) return true;
        }
        return false;
    }
    /* Thread for get time from Service */
    private class GetTimeThread implements Runnable {
        int time_sec;
        /* Need handler to get time from binder. */
        Handler handler = new Handler();
        @Override
        public void run() {
            while(!gameover){
                if(!isServiceOn) continue; /* If disconnected. */
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        try{
                            /* Get time from binder.
                             * Set MM:SS data in clock textView */
                            time_sec = ms.getTime();
                            System.out.println("time_sec :" + time_sec);
                            String str = String.format("%02d:%02d", time_sec/60, time_sec%60);
                            clock.setText(str);
                        } catch (Exception e){
                            e.printStackTrace();
                        }
                    }
                });
                try{ /* Check interval */
                    Thread.sleep(100);
                } catch (InterruptedException e){
                    e.printStackTrace();
                }
            }
        }
    }
}
