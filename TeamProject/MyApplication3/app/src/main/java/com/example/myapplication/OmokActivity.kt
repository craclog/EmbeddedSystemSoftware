package com.example.myapplication

import android.graphics.*
import android.graphics.drawable.shapes.OvalShape
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import kotlinx.android.synthetic.main.activity_omok.*
import kotlin.math.max
import kotlin.math.min

val USER_ID = 1

val LINE_INTERVAL : Float = 25.0f
val LINE_NUM : Int = 19
val BOARD_SIZE : Int = (LINE_NUM + 1) * LINE_INTERVAL.toInt() + 1
val LINE_LENGTH : Float = (LINE_NUM - 1) * LINE_INTERVAL
//val MY_STONE = 1
//val OPPONENT_STONE = 2

var stoneInfo = Array(LINE_NUM, {IntArray(LINE_NUM)})

class OmokActivity : AppCompatActivity() {

    var myTurn : Boolean = if(USER_ID == 1){
        true
    } else{
        false
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_omok)
        println("Received ID : " + intent.getStringExtra("id"))
        myID_textView.text = intent.getStringExtra("id")

        HighlightUser()
        drawBoard()

        /* Set Buttons' onClickListeners */
        ok_btn.setOnClickListener {
//            if(myTurn) stoneInfo[LockOn.rowIdx][LockOn.colIdx] = USER_ID
            stoneInfo[LockOn.rowIdx][LockOn.colIdx] = USER_ID
            drawBoard()
            myTurn = !myTurn
            HighlightUser()
        }
        left_btn.setOnClickListener {
            LockOn.rowIdx = max(LockOn.rowIdx - 1, 0)
            drawBoard()
        }
        up_btn.setOnClickListener {
            LockOn.colIdx = max(LockOn.colIdx - 1, 0)
            drawBoard()
        }
        right_btn.setOnClickListener {
            LockOn.rowIdx = min(LockOn.rowIdx + 1, LINE_NUM - 1)
            drawBoard()
        }
        down_btn.setOnClickListener {
            LockOn.colIdx = min(LockOn.colIdx + 1, LINE_NUM - 1)
            drawBoard()
        }
    }
    fun HighlightUser(){
        /* If it's my turn, paint myID_textView Yellow.
         * else paint OpID_textView Yellow. */
        if(myTurn) {
            myID_textView.setBackgroundColor(Color.YELLOW)
            OpID_textView.setBackgroundColor(Color.WHITE)
        } else {
            myID_textView.setBackgroundColor(Color.WHITE)
            OpID_textView.setBackgroundColor(Color.YELLOW)
        }
    }
    fun drawBoard(){
        val bitmap : Bitmap = Bitmap.createBitmap(BOARD_SIZE, BOARD_SIZE, Bitmap.Config.ARGB_8888)
        val canv = Canvas(bitmap)
        val paint = PaintBoard()

        val backgroundRect = Rect(LINE_INTERVAL.toInt(), LINE_INTERVAL.toInt(),
            BOARD_SIZE - LINE_INTERVAL.toInt(), BOARD_SIZE - LINE_INTERVAL.toInt())
        //#FAAC38
        paint.color = Color.parseColor("#faac38")
        paint.style = Paint.Style.FILL_AND_STROKE
        canv.drawRect(backgroundRect, paint)
        paint.color = Color.BLACK
        paint.style = Paint.Style.STROKE


        for(i in 1..LINE_NUM) {
            val x: Float = i * LINE_INTERVAL
            if( i == 10) paint.strokeWidth = 2f
            canv.drawLine(x, LINE_INTERVAL, x, LINE_INTERVAL + LINE_LENGTH, paint)
            canv.drawLine(LINE_INTERVAL, x, LINE_INTERVAL + LINE_LENGTH, x, paint)
            if( i == 10) paint.strokeWidth = 1f
        }
        for(i in 0 until LINE_NUM){
            for(j in 0 until LINE_NUM){
                println("("+i+","+ j + ") : " + stoneInfo[i][j])
                if(stoneInfo[i][j] == 1){
                    val rect = getOval(i, j)
                    paint.style = Paint.Style.FILL_AND_STROKE
                    canv.drawOval(rect, paint)
                    paint.style = Paint.Style.STROKE
                } else if(stoneInfo[i][j] == 2){
                    val rect = getOval(i, j)
                    paint.style = Paint.Style.FILL_AND_STROKE
                    paint.color = Color.WHITE
                    canv.drawOval(rect, paint)
                    paint.style = Paint.Style.STROKE
                    paint.color = Color.BLACK
                }
            }
        }
        LockOn.calcRect()
        paint.color = Color.RED
        canv.drawRect(LockOn.rect, paint)
        imageView.setImageBitmap(bitmap)
    }
    fun getOval(line_x : Int, line_y : Int) : RectF { // line_x : 0~18
        val x = LINE_INTERVAL + line_x * LINE_INTERVAL
        val y = LINE_INTERVAL + line_y * LINE_INTERVAL

        val left = (x - LINE_INTERVAL/2)
        val top = (y - LINE_INTERVAL/2)
        val right = (x + LINE_INTERVAL/2)
        val bottom = (y + LINE_INTERVAL/2)
        return RectF(left, top, right, bottom)
    }
    class PaintBoard : Paint(){
        init{
            this.color = Color.BLACK
            this.strokeWidth = 1.0f
            this.style = Style.STROKE
        }
    }
    private inner class Reader : Runnable{
        override fun run() {

        }
    }
    object LockOn {
        var rowIdx : Int = LINE_NUM / 2 // rowIdx : 0 ~ 18
        var colIdx : Int = LINE_NUM / 2
        var x_coord : Float = 0f
        var y_coord : Float = 0f
        lateinit var  rect : Rect

        fun initPosition(){ // Center idx : (9, 9)
            rowIdx = LINE_NUM / 2
            colIdx = LINE_NUM / 2
        }
        fun calcCoord(){
            x_coord = LINE_INTERVAL + rowIdx * LINE_INTERVAL
            y_coord = LINE_INTERVAL + colIdx * LINE_INTERVAL
            println("(x,y) : " + x_coord + "," + y_coord)
        }
        fun calcRect(){
            calcCoord()
            val left : Int = (x_coord - LINE_INTERVAL/2).toInt()
            val top : Int = (y_coord - LINE_INTERVAL/2).toInt()
            val right : Int = (x_coord + LINE_INTERVAL/2).toInt()
            val bottom : Int = (y_coord + LINE_INTERVAL/2).toInt()
            rect = Rect(left, top, right, bottom)
        }
    }
}
