#include "20131579.h"

key_t outputq_keyid, inputq_keyid;
int mode = DEFAULT_MODE;
input_buf i_msg;
output_buf o_msg; 

unsigned char dot_data[12][10] = {
	{0x3e,0x7f,0x63,0x73,0x73,0x6f,0x67,0x63,0x7f,0x3e}, // 0
	{0x0c,0x1c,0x1c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x1e}, // 1
	{0x7e,0x7f,0x03,0x03,0x3f,0x7e,0x60,0x60,0x7f,0x7f}, // 2
	{0xfe,0x7f,0x03,0x03,0x7f,0x7f,0x03,0x03,0x7f,0x7e}, // 3
	{0x66,0x66,0x66,0x66,0x66,0x66,0x7f,0x7f,0x06,0x06}, // 4
	{0x7f,0x7f,0x60,0x60,0x7e,0x7f,0x03,0x03,0x7f,0x7e}, // 5
	{0x60,0x60,0x60,0x60,0x7e,0x7f,0x63,0x63,0x7f,0x3e}, // 6
	{0x7f,0x7f,0x63,0x63,0x03,0x03,0x03,0x03,0x03,0x03}, // 7
	{0x3e,0x7f,0x63,0x63,0x7f,0x7f,0x63,0x63,0x7f,0x3e}, // 8
	{0x3e,0x7f,0x63,0x63,0x7f,0x3f,0x03,0x03,0x03,0x03}, // 9
	{0x08,0x1C,0x36,0x36,0x77,0x7F,0x7F,0x63,0x63,0x63}, // A
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}	 //CLEAR
};

/* MODE_CLOCK */
extern int status_mode1_changing;
extern struct timeval curtime, newtime, blinktime, nexttime, tmp_time;
/* MODE_COUNTER */
extern int cnt2;
extern int base;
/* MODE_TEXTEDITOR */
extern int a1;
extern int cnt3;
extern int last_sw;
extern int streak;
extern int length;
extern unsigned char strbuf[MAX_STR_BUFF + 1];
extern const char button[10][3];
/* MODE_DRAWBOARD */
extern int cnt4;
extern int cur_r, cur_c;
extern unsigned char hex_dot[10];
extern int cur_visible;
extern struct timeval next_blink_time, m4_cur_time;



// send msg
void send_fnd(int data){
	o_msg.mtype = OUTPUTQ_KEY;
	o_msg.fix_bit = FIX_FND;
	num2array(data, o_msg.fnd);
	// printf("proc::send fnd::outkey : %d\n", outputq_keyid);
	if(msgsnd(outputq_keyid, (void *)&o_msg, sizeof(output_buf) - sizeof(long), IPC_NOWAIT)) {
		perror("reader::msgsnd error: ");
		exit(1);
	}
}
void send_led(int data){
	o_msg.mtype = OUTPUTQ_KEY;
	o_msg.fix_bit = FIX_LED;
	o_msg.led = (unsigned char)num2led(data);
	if(msgsnd(outputq_keyid, (void *)&o_msg, sizeof(output_buf) - sizeof(long), IPC_NOWAIT)) {
		perror("reader::msgsnd error: ");
		exit(1);
	}
}
void send_lcd(unsigned char str[MAX_STR_BUFF]){
	printf("%s\n", str);
	o_msg.mtype = OUTPUTQ_KEY;
	o_msg.fix_bit = FIX_LCD;
	strcpy(o_msg.text, str);
	if(msgsnd(outputq_keyid, (void *)&o_msg, sizeof(output_buf) - sizeof(long), IPC_NOWAIT)) {
		perror("reader::msgsnd error: ");
		exit(1);
	}
}
void send_dot(unsigned char data[10]){
	int i;
	o_msg.mtype = OUTPUTQ_KEY;
	o_msg.fix_bit = FIX_DOT;
	for(i=0; i<DOT_HEIGHT; i++)
		o_msg.hex_dot[i] = data[i];
	if(msgsnd(outputq_keyid, (void *)&o_msg, sizeof(output_buf) - sizeof(long), IPC_NOWAIT)) {
		perror("reader::msgsnd error: ");
		exit(1);
	}
}

void clear_mode(int mode){
	if(mode == MODE_CLOCK){
		send_fnd(0);
		send_led(0);
	} else if(mode == MODE_COUNTER){
		send_fnd(0);
		send_led(0);
	} else if(mode == MODE_TEXTEDITOR) {
		clear_text(strbuf);
		send_fnd(0);
		send_lcd(strbuf);
		send_dot(dot_data[DOT_CLEAR]);
	} else if(mode == MODE_DRAWBOARD) {
		send_fnd(0);
		send_dot(dot_data[DOT_CLEAR]);
		cur_visible = HIDE_ON_BUSH;
	} else if(mode == 5) {
		//TODO
	}
}

// init mode
void init_mode(int mode){
	if(mode == MODE_CLOCK){
		int data = sec2clock(curtime.tv_sec);
		send_fnd(data);
		send_led(1);
		status_mode1_changing = 0;
	} else if(mode == MODE_COUNTER){
		cnt2 = 0;
		base = 10;
		send_fnd(cnt2);
		send_led(BASE_10);
	} else if(mode == MODE_TEXTEDITOR){
		a1 = ALPHA;
		cnt3 = 0;
		streak = 0;
		last_sw = -1;
		length = 0;
		clear_text(strbuf);
		send_dot(dot_data[DOT_A]);
	} else if(mode == MODE_DRAWBOARD){
		cnt4 = 0;
		cur_r = cur_c = 0;
		cur_visible = NO_HIDE;
		clear_dot();
		gettimeofday(&m4_cur_time, NULL);
        next_blink_time.tv_sec = m4_cur_time.tv_sec;
	}
}
int proc_main(){
	gettimeofday(&curtime, NULL);
	nexttime.tv_sec = curtime.tv_sec + 60;
	o_msg.mtype = OUTPUTQ_KEY;
	struct msqid_ds buf;

	printf("Current Sys Time : %02d:%02d\n", curtime.tv_sec/3600, (curtime.tv_sec%3600)/60);
	
	// msgctl((key_t)INPUTQ_KEY, IPC_RMID, 0);
	// msgctl((key_t)OUTPUTQ_KEY, IPC_RMID, 0);

	// make message queue for input process
	inputq_keyid = msgget((key_t)INPUTQ_KEY, IPC_CREAT|0666);
	if(inputq_keyid == -1){
		perror("processor::msgget error : ");
		exit(1);
	}
	// make message queue for output process
	outputq_keyid = msgget((key_t)OUTPUTQ_KEY, IPC_CREAT|0666);
	if(outputq_keyid == -1){
		perror("processor::msgget error : ");
		exit(1);
	}
	usleep(10000);

	//initialize devices
	clear_text(strbuf);
	send_fnd(0);
	send_led(0);
	send_lcd(strbuf);
	send_dot(dot_data[DOT_CLEAR]);

	//Default mode
	init_mode(MODE_CLOCK);
	
	while(1){
		// periodic task
		if(mode == MODE_CLOCK) mode1_period();
		mode4_blink();
		
		// check msg_q num
		int rc = msgctl(inputq_keyid, IPC_STAT, &buf);
		int msg_num = (int)(buf.msg_qnum);
		if(msg_num <= 0) continue;	//if empty, continue	
		if(msgrcv(inputq_keyid, (void *)&i_msg, sizeof(input_buf) - sizeof(long), 0, 0)	== -1){
			perror("processor::msgrcv error:");
			exit(1);
		}
		// receive function key
		if(i_msg.type == FUNCTION_KEY){
			printf("proc::func rcv\n");
			if(i_msg.ev.code == BACK){
				//TODO

				//kill children processes
				//close all devices
				//munmap
				//Die
			} else if(i_msg.ev.code == VOL_UP) {
				clear_mode(mode);
				mode = mode % MAX_MODE_NUM + 1;	
				init_mode(mode);
			} else if(i_msg.ev.code == VOL_DOWN) {
				clear_mode(mode);
				mode -= 1;
				if(mode == 0) mode = MAX_MODE_NUM;				
				init_mode(mode);
			}
			printf("proc::Current mode = %d\n", mode);
		}
		// receive switch key
		else if(i_msg.type == SWITCH_KEY){
			printf("proc::swit rcv, sw_num:%d\n", i_msg.sw_num);
			if(mode == MODE_CLOCK){			
				if(i_msg.sw_num == 2);
				else if(i_msg.sw_id1 == 1){	// change time
					mode1_change_time();
				} else if(i_msg.sw_id1 == 2){ // reset time
					mode1_reset_time();
				} else if(i_msg.sw_id1 == 3 && status_mode1_changing){ // +1 Hour
					mode1_add_hour();
				} else if(i_msg.sw_id1 == 4 && status_mode1_changing){ // +1 min
					mode1_add_min();
				}
			} else if(mode == MODE_COUNTER){
				if(i_msg.sw_num == 2);
				else if(i_msg.sw_id1 == 1){	// change base(10-8-4-2-10)
					mode2_change_base();
				} else if(i_msg.sw_id1 == 2){ // +0100
					mode2_add(100);
				} else if(i_msg.sw_id1 == 3){ // +0010
					mode2_add(10);
				} else if(i_msg.sw_id1 == 4){ // +0001
					mode2_add(1);
				}
			} else if(mode == MODE_TEXTEDITOR){
				if(i_msg.sw_num == 1){	// edit text
					cnt3++;
					mode3_switch1(i_msg.sw_id1);
				} else if(i_msg.sw_num == 2){
					if(i_msg.sw_id1 == 2 && i_msg.sw_id2 == 3){ // clear text
						cnt3++;
						clear_text(strbuf);				
					} else if(i_msg.sw_id1 == 5 && i_msg.sw_id2 == 6){ // change input mode(a-123)
						cnt3++;
						mode3_change_input_mode();
					} else if(i_msg.sw_id1 == 8 && i_msg.sw_id2 == 9){ // insert space(' ')
						cnt3++;
						mode3_insert_char(' ', NO_OVERWRITE);
						streak = 0;
						last_sw = -1;
					}
				}
				cnt3 %= 10000;
				send_fnd(cnt3);
				send_lcd(strbuf);			 
			} else if(mode == MODE_DRAWBOARD){
				if(i_msg.sw_num == 2);
				else {
					cnt4 = (cnt4 + 1) % 10000;
					mode4_switch(i_msg.sw_id1);				
					send_fnd(cnt4);
					send_dot(hex_dot);
				}
			} else if(mode == 5){

			}
		}
	}


	return 0;
}

