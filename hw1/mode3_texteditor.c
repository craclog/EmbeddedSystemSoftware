#include "20131579.h"

/* MODE_TEXTEDITOR */

int a1; /* input mode: Alphabet or Number */
int cnt3;
int last_sw;
int streak; /* # of clicks same switch in row */
int length; /* Current text length */
unsigned char strbuf[MAX_STR_BUFF + 1];
extern unsigned char dot_data[12][10];

const char button[10][3] = {
    {0, 0, 0},          //padding
	{'.', 'Q', 'Z'},	// 1
	{'A', 'B', 'C'},	// 2
	{'D', 'E', 'F'},	// 3
	{'G', 'H', 'I'},	// 4
	{'J', 'K', 'L'},	// 5
	{'M', 'N', 'O'},	// 6
	{'P', 'R', 'S'},	// 7
	{'T', 'U', 'V'},	// 8
	{'W', 'X', 'Y'}		// 9
};
/*
 * Change input mode (alpha <-> Num)
 */
void mode3_change_input_mode(){
    if(a1 == ALPHA){
    	a1 = NUMBER;
	    send_dot(dot_data[1]);
	} else if(a1 == NUMBER){
		a1 = ALPHA;
		send_dot(dot_data[DOT_A]);
	}
	streak = 0;
	last_sw = -1;
}
/*
 * Clear text
 */
void clear_text(unsigned char str[MAX_STR_BUFF + 1]){
	memset(str, ' ', MAX_STR_BUFF);
	str[MAX_STR_BUFF] = 0;
	length = 0;
    streak = 0;
    last_sw = -1;
}
/*
 * input: Char to insert, overwrite mode
 * Insert new Character to text.
 * if text buffer is full, oldest char will be erased.
 */
void mode3_insert_char(char c, int overwrite){
	int i;
	if(overwrite){
		strbuf[length - 1] = c;
	} else{
		if(length == MAX_STR_BUFF){
			for(i=0; i<length - 1; i++){
				strbuf[i] = strbuf[i+1];
			}
			strbuf[i] = c;
		} else{
			strbuf[length] = c;
			length++;
		}
	}
	strbuf[MAX_STR_BUFF] = 0;
}
/*
 * input: switch id
 */
void mode3_switch1(int sw_id){
	char c;
	if(a1 == ALPHA){ /* insert alphabet */
		if(last_sw == sw_id) { 
			streak++;
			c = button[sw_id][streak % 3];
			mode3_insert_char(c, OVERWRITE);
		}
		else { /* new character */
			streak = 0;
			c = button[sw_id][streak % 3];
			mode3_insert_char(c, NO_OVERWRITE);
		}
	} else if(a1 == NUMBER){ /* insert num */
		c = sw_id + '0';
		mode3_insert_char(c, NO_OVERWRITE);
	}
	last_sw = sw_id;
}