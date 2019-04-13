#include "20131579.h"

/* MODE_DRAWBOARD */

int cnt4;
int cur_r, cur_c;
int cur_visible;
unsigned char map[10][8];
unsigned char hex_dot[10];
struct timeval next_blink_time, m4_cur_time;

/*
 * input: cursor's row, column
 * return true if (r,c) is valid, else return false
 */
int boundary_chk(int r, int c){
	if(r < 0 || r >= DOT_HEIGHT || c < 0 || c >= DOT_WIDTH) return 0;
	return 1;
}
/*
 * input: hex data[10], row, column, command
 * Select bit or deselect where cursor on
 */
void select_hex_dot(unsigned char data[10], int r, int c, int cmd){
	int i;
	unsigned char val;
	if(c == 0) val = 0x40;
	else if(c == 1) val = 0x20;
	else if(c == 2) val = 0x10;
	else if(c == 3) val = 0x08;
	else if(c == 4) val = 0x04;
	else if(c == 5) val = 0x02;
	else if(c == 6) val = 0x01;
    
    if(cmd == DOT_NOSEL) data[r] &= (0x7f ^ val);
    else if(cmd == DOT_SEL) data[r] |= val;
	else if(cmd == DOT_XOR) data[r] ^= val;
}
/*
 * if cursor is visible, cursor blinks per 1 sec
 */
void mode4_blink(){
	int i;
	if(cur_visible == HIDE_ON_BUSH){
		/* Faker */
	} else if(cur_visible == NO_HIDE){ /* blink */
		gettimeofday(&m4_cur_time, NULL);
		if(m4_cur_time.tv_sec >= next_blink_time.tv_sec){ /* send dot msg per 1 sec */
			unsigned char tmp[10]; 
			for(i=0; i< DOT_HEIGHT; i++)
				tmp[i] = hex_dot[i];
			next_blink_time.tv_sec = m4_cur_time.tv_sec + 1; /* set next blink time */
			if(m4_cur_time.tv_sec % 2 == 0){
				select_hex_dot(tmp, cur_r, cur_c, DOT_SEL);
				send_dot(tmp);
			} else{
				select_hex_dot(tmp, cur_r, cur_c, DOT_NOSEL);
				send_dot(tmp);
			}
		}
	}
}
/*
 * Clear hex dot to 0x00
 */
void clear_dot(){
	int i;
	for(i=0; i<10; i++){
		hex_dot[i] = 0x00;
	}
}
/*
 * Inverse hex dot
 */
void inverse_dot(){
	int i;
	for(i=0; i<10; i++){
		hex_dot[i] ^= 0x7f;
	}
}
/*
 * Change hide mode. Set blink time
 */
void mode4_hide_cursor(){
	if(cur_visible == HIDE_ON_BUSH){
		cur_visible = NO_HIDE;
   		gettimeofday(&m4_cur_time, NULL);
        next_blink_time.tv_sec = m4_cur_time.tv_sec;
	} else if(cur_visible == NO_HIDE){
		cur_visible = HIDE_ON_BUSH;
        send_dot(hex_dot);
	}
}
/*
 * input: switch id
 * mode4 function
 */
void mode4_switch(int sw_id){
	if(sw_id == 1){ /* reset mode */
		cnt4 = 0;
		cur_r = cur_c = 0;
		cur_visible = NO_HIDE;
		clear_dot();
        gettimeofday(&m4_cur_time, NULL);
        next_blink_time.tv_sec = m4_cur_time.tv_sec;
	} else if(sw_id == 2){ /* cursor UP */
		if(boundary_chk(cur_r - 1, cur_c)) cur_r -= 1;
	} else if(sw_id == 3){ /* hide cursor */
		mode4_hide_cursor();
	} else if(sw_id == 4){ /* cursor LEFT */
		if(boundary_chk(cur_r, cur_c - 1)) cur_c -= 1;
	} else if(sw_id == 5){ /* Select or Deselect bit where cursor on */
		select_hex_dot(hex_dot, cur_r, cur_c, DOT_XOR);
	} else if(sw_id == 6){ /* cursor RIGHT */
		if(boundary_chk(cur_r, cur_c + 1)) cur_c += 1;
	} else if(sw_id == 7){ /* clear dot */
		clear_dot();
	} else if(sw_id == 8){ /* cursor DOWN */
		if(boundary_chk(cur_r + 1, cur_c)) cur_r += 1;
	} else if(sw_id == 9){ /* inverse dot */
		inverse_dot();
	}
}