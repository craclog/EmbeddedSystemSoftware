package com.example.puzzle;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

/* Initial Activity for Puzzle Application.
 * If you click "START PUZZLE" button, game starts.
 * */
public class MainActivity extends AppCompatActivity {

    Button start_btn;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        /* Find start Button. */
        start_btn = findViewById(R.id.start_btn);
        /* OnClickListener for start_btn (START PUZZLE) */
        View.OnClickListener listener = new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                /* Go to Puzzle game Activity */
                Intent intent = new Intent(MainActivity.this, PuzzleActivity.class);
                startActivity(intent);
            }
        };
        start_btn.setOnClickListener(listener);
    }
}
