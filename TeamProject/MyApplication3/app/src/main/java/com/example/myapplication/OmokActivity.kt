package com.example.myapplication

import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import kotlinx.android.synthetic.main.activity_omok.*


class OmokActivity : AppCompatActivity() {
    val LINE_INTERVAL : Float = 25.0f
    val LINE_NUM : Int = 19
    val BOARD_SIZE : Int = (LINE_NUM + 1) * LINE_INTERVAL.toInt() + 1
    val LINE_LENGTH : Float = (LINE_NUM - 1) * LINE_INTERVAL

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_omok)
        val bitmap : Bitmap = Bitmap.createBitmap(BOARD_SIZE, BOARD_SIZE, Bitmap.Config.ARGB_8888)
        val canv = Canvas(bitmap)
        val paint = PaintBoard()

        for(i in 1..LINE_NUM){
            val x : Float = i * LINE_INTERVAL
            canv.drawLine(x, LINE_INTERVAL, x, LINE_INTERVAL + LINE_LENGTH, paint)
            canv.drawLine(LINE_INTERVAL, x, LINE_INTERVAL + LINE_LENGTH, x, paint)
        }


        imageView.setImageBitmap(bitmap)
//        canv.drawColor(0xff9933)


    }

    class PaintBoard : Paint(){
        init{
            this.color = Color.GRAY
            this.strokeWidth = 1.0f
            this.style = Style.FILL
        }
    }
    private inner class Reader : Runnable{
        override fun run() {

        }
    }
}
