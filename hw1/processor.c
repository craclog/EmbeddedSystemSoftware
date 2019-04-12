#include "20131579.h"

key_t outputq_keyid, inputq_keyid;
int mode = DEFAULT_MODE;
input_buf i_msg;
output_buf o_msg; 

/* MODE_CLOCK */
extern int status_mode1_changing;
extern struct timeval curtime, newtime, blinktime, nexttime, tmp_time;
/* MODE_COUNTER */
extern int cnt2;
extern int base;
/* MODE_TEXTEDITOR */


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

void clear_mode(int mode){
	if(mode == MODE_CLOCK){
		send_fnd(0);
		send_led(0);
	} else if(mode == MODE_COUNTER){
		send_fnd(0);
		send_led(0);
	} else if(mode == MODE_TEXTEDITOR) {
		send_fnd(0);
		// init_text();
		// init_dot();
	} else if(mode == MODE_DRAWBOARD) {
		// init_fnd();
		// init_dot();
	} else if(mode == 5) {
		//TODO
	}
}

// init mode
void init_mode(int mode){
	if(mode == MODE_CLOCK){
		printf("init mode 1\n");
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

	}
}
int proc_main(){
	gettimeofday(&curtime, NULL);
	nexttime.tv_sec = curtime.tv_sec + 60;
	o_msg.mtype = OUTPUTQ_KEY;
	struct msqid_ds buf;

	printf("time :   %02d:%02d\n", curtime.tv_sec/3600, (curtime.tv_sec%3600)/60);
	
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

	// TODO
	//initialize devices
	send_fnd(0);
	send_led(0);

	//Default mode == MODE_CLOCK
	init_mode(MODE_CLOCK);
	
	while(1){
		// periodic task
		if(mode == MODE_CLOCK) mode1_period();
		
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
		}
		// receive switch key
		else if(i_msg.type == SWITCH_KEY){
			printf("proc::swit rcv\n");
			if(mode == MODE_CLOCK){			
				if(i_msg.sw_id1 == 1){	// change time
					mode1_change_time();
				} else if(i_msg.sw_id1 == 2){ // reset time
					mode1_reset_time();
				} else if(i_msg.sw_id1 == 3 && status_mode1_changing){ // +1 Hour
					mode1_add_hour();
				} else if(i_msg.sw_id1 == 4 && status_mode1_changing){ // +1 min
					mode1_add_min();
				}
			} else if(mode == MODE_COUNTER){
				if(i_msg.sw_id1 == 1){	// change base(10-8-4-2-10)
					mode2_change_base();
				} else if(i_msg.sw_id1 == 2){ // +0100
					mode2_add(100);
				} else if(i_msg.sw_id1 == 3){ // +0010
					mode2_add(10);
				} else if(i_msg.sw_id1 == 4){ // +0001
					mode2_add(1);
				}
			} else if(mode == MODE_TEXTEDITOR){

			} else if(mode == MODE_DRAWBOARD){

			} else if(mode == 5){

			}
		}
	}


	return 0;
}

