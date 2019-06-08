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
import java.util.Collections;
import java.util.ArrayList;

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
    ArrayList<Integer> btn_order = new ArrayList<>();

    private boolean gameover = false;
    private int row, col;
    private int blank_row, blank_col, blank_idx;
    int btn_width, btn_height;
    private IMyTimerInterface binder = null;

    private ServiceConnection connection  = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            binder = IMyTimerInterface.Stub.asInterface(service);
        }
        @Override
        public void onServiceDisconnected(ComponentName name) {
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_puzzle);
        make_btn = findViewById(R.id.make_btn);
        data = findViewById(R.id.input_plainText);
        main_layout = findViewById(R.id.btn_main_LinearLayout);
        clock = findViewById(R.id.time_textView);
        context = this;

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

                    generate_btn_order();  /* Generate random buttons' order */
                    generate_btns(); /* Make buttons dynamically */

                    /* Start Timer */
                    Intent intent_timer = new Intent(PuzzleActivity.this, MyTimerService.class);
                    if(binder != null) unbindService(connection);
                    bindService(intent_timer, connection, BIND_AUTO_CREATE);
                    gameover = false;
                    new Thread(new GetTimeThread()).start();

                } catch (Exception e){
                    /* If splitted data's length > 2, or row == 0, or col == 0 */
                    Toast.makeText(getApplicationContext(), "Invalid input data [row(1~5) col(1~5)]", Toast.LENGTH_LONG).show();
                    if(binder != null) {
                        unbindService(connection); /* Disconnect binder */
                        binder = null;
                    }
                    gameover = true; // To finish GetTimeThread
                    clock.setText("00:00"); // Init clock TextView
                    System.out.println("Make_btn listener Exception occurred! : " + e);
                }
            }
        };
        make_btn.setOnClickListener(listener);
    }

    public void clear_layout(){
        main_layout.removeAllViews();
    }
    public int calc_btn_width(){

        int w = main_layout.getWidth();
        int ret = (w - (col - 1)*PADDING) / col;
        return ret;
    }
    public int calc_btn_height(){

        int h = main_layout.getHeight();
        int ret = (h - (row - 1)*PADDING) / row;
        return ret;
    }
    public void generate_btn_order(){
        if(!btn_order.isEmpty())
            btn_order.clear();
        for (int i = 0; i < row*col; i++)
            btn_order.add(i);
        /* Shuffle button number and set btns_info[] */
        Collections.shuffle(btn_order);
        for(int i=0; i<row*col; i++)
            btns_info[i] = btn_order.get(i);

        /* To prevent Initial state is win-game, shuffle again. */
        while(row*col > 1 && is_end_game()){
            Collections.shuffle(btn_order);
            for(int i=0; i<row*col; i++)
                btns_info[i] = btn_order.get(i);
        }
    }
    View.OnClickListener btn_listener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {

            int btn_idx = v.getId();
            int num_on_btn = btns_info[btn_idx];

            if(is_valid_move(v, btn_idx)){
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
    public boolean is_end_game(){
        int num_of_btn = row * col - 1;
        for(int i=0; i<num_of_btn; i++){
            if(btns_info[i] != i + 1) return gameover = false   ;
        }
        return gameover = true;
    }
    public int id2row(int id){
        return id / col;
    }
    public int id2col(int id){
        return id % col;
    }
    public boolean is_valid_move(View v, int btn_idx){

        int btn_row = id2row(btn_idx);
        int btn_col = id2col(btn_idx);

        if(btn_idx == blank_idx) return false;
        else if(btn_row == blank_row){
            if( abs(btn_col - blank_col) <= 1) return true;
        }
        else if(btn_col == blank_col){
            if( abs(btn_row - blank_row) <= 1) return true;
        }
        return false;
    }
    private class GetTimeThread implements Runnable {
        int time_sec;
        Handler handler = new Handler();
        @Override
        public void run() {
            while(!gameover){
                if(binder == null) continue;
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        try{
                            time_sec = binder.getTime();
                            String str = String.format("%02d:%02d", time_sec/60, time_sec%60);
                            clock.setText(str);
                        } catch (Exception e){
                            e.printStackTrace();
                        }
                    }
                });

                try{
                    Thread.sleep(100);
                } catch (InterruptedException e){
                    e.printStackTrace();
                }
            }
        }
    }
}
