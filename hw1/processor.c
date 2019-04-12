#include "20131579.h"

key_t outputq_keyid, inputq_keyid;
int mode = DEFAULT_MODE;
input_buf i_msg;
output_buf o_msg; 

/* MODE_CLOCK */
int status_mode1_changing = 0;
struct timeval curtime, newtime, blinktime, nexttime, tmp_time;

void mode1_period(){
	if(status_mode1_changing){
		// send led msg per 1 sec
		gettimeofday(&tmp_time, NULL);
		if(tmp_time.tv_sec >= blinktime.tv_sec){
			blinktime.tv_sec = tmp_time.tv_sec + 1;
			if(tmp_time.tv_sec % 2 == 0){
				send_led(3);
			} else{
				send_led(4);
			}
		}
	} else{
		// send time per 1 min
		gettimeofday(&tmp_time, NULL);
		if(tmp_time.tv_sec >= nexttime.tv_sec) {
			nexttime.tv_sec = tmp_time.tv_sec + 60;
			curtime.tv_sec += 60;
			int data = sec2clock(curtime.tv_sec);
			send_fnd(data);
		}
	}
}


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
		// init_fnd();
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
		mode1_period();
		
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
			//printf("CUR MODE : %d\n", mode);
		}
		// receive switch key
		else if(i_msg.type == SWITCH_KEY){
			printf("proc::swit rcv\n");
			if(mode == MODE_CLOCK){			
				if(i_msg.sw_id1 == 1){	// change time
					if(!status_mode1_changing) {
						status_mode1_changing = 1;
						newtime = curtime;
						blinktime.tv_sec = curtime.tv_sec + 1;
						nexttime.tv_sec = curtime.tv_sec + 60;
					} else {
						status_mode1_changing = 0;
						gettimeofday(&curtime, NULL);
						nexttime.tv_sec = curtime.tv_sec + 60;
						curtime = newtime;
						int data = sec2clock(curtime.tv_sec);
						send_fnd(data);
						send_led(1);
					}
				} else if(i_msg.sw_id1 == 2){ // reset time
					status_mode1_changing = 0;
					gettimeofday(&curtime, NULL);
					nexttime.tv_sec = curtime.tv_sec + 60;
					int data = sec2clock(curtime.tv_sec);
					send_fnd(data);
					send_led(1);
				} else if(i_msg.sw_id1 == 3 && status_mode1_changing){ // +1 Hour
					newtime.tv_sec += 3600;
					int data = sec2clock(newtime.tv_sec);
					send_fnd(data);
				} else if(i_msg.sw_id1 == 4 && status_mode1_changing){ // +1 min
					newtime.tv_sec += 60;
					int data = sec2clock(newtime.tv_sec);
					send_fnd(data);
				}
			} else if(mode == MODE_COUNTER){
				
			} else if(mode == MODE_TEXTEDITOR){

			} else if(mode == MODE_DRAWBOARD){

			} else if(mode == 5){

			}
		}
	}


	return 0;
}

