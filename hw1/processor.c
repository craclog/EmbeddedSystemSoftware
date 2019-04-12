#include "mque.h"

#define DEFAULT_MODE 1
#define MAX_MODE_NUM 4	// add to 5

#define BACK 158
#define VOL_UP 115
#define VOL_DOWN 114

int mode = DEFAULT_MODE;
key_t inputq_keyid, outputq_keyid;	
input_buf i_msg;
output_buf o_msg; 

// util
void num2array(int num, char* arr){
	int i;
	for(i=3; i>=0; i--){
		arr[i] = num % 10; 
		num /= 10;
	}
}
//TODO
void init_fnd(){
	o_msg.mtype = OUTPUTQ_KEY;
	o_msg.fix_bit = FIX_FND;
	num2array(0, o_msg.fnd);
	if(msgsnd(outputq_keyid, (void *)&o_msg, sizeof(output_buf), IPC_NOWAIT)) {
		perror("reader::msgsnd error: ");
		exit(1);
	}
}
void init_led(){
	o_msg.mtype = OUTPUTQ_KEY;
	o_msg.fix_bit = FIX_LED;
	o_msg.led = 0;
	if(msgsnd(outputq_keyid, (void *)&o_msg, sizeof(output_buf), IPC_NOWAIT)) {
		perror("reader::msgsnd error: ");
		exit(1);
	}
}
void init_text(){
}
void init_dot(){
}

void clear_mode(int mode){
	if(mode == 1){
		init_fnd();
		init_led();
	} else if(mode == 2){
		init_fnd();
		init_led();
	} else if(mode == 3) {
		init_fnd();
		init_text();
		init_dot();
	} else if(mode == 4) {
		init_fnd();
		init_dot();
	} else if(mode == 5) {
		//TODO
	}
}


int main(){
	pid_t reader_pid, writer_pid;
	

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
	// make input(reader) process
	if((reader_pid = fork()) == -1){
		perror("reader fork error:");
		exit(1);
	}
	if(reader_pid == 0){
		char *argv[] = {"./reader"};
		execv(argv[0], argv);
	}
	// make output(writer) process
	if((writer_pid = fork()) == -1){
		perror("writer fork error:");
		exit(1);
	}
	if(writer_pid == 0){
		char *argv[] = {"./writer"};
		execv(argv[0], argv);
	}
	//initialize devices
	// TODO
	init_fnd();
	init_led();
	init_text();
	init_dot();


	while(1){
		if(msgrcv(inputq_keyid, (void *)&i_msg, sizeof(input_buf), INPUTQ_KEY, 0)
				== -1){
			perror("processor::msgrcv error:");
			exit(1);
		}
		// receive function key
		if(i_msg.type == FUNCTION_KEY){
			if(i_msg.ev.code == BACK){
				//TODO
				
				//kill children processes
				//close all devices
				//munmap
				//Die
			} else if(i_msg.ev.code == VOL_UP) {
				clear_mode(mode);
				mode = mode % MAX_MODE_NUM + 1;			
			} else if(i_msg.ev.code == VOL_DOWN) {
				clear_mode(mode);
				mode -= 1;
				if(mode == 0) mode = MAX_MODE_NUM;				
			}
			printf("CUR MODE : %d\n", mode);
		}
		// receive switch key
		else if(i_msg.type == SWITCH_KEY){
			// printf("processor::Switch_key, sw_num:%d, sw_id1:%d", i_msg.sw_num, i_msg.sw_id1);
			// if(i_msg.sw_num == 2) printf(", sw_id2:%d", i_msg.sw_id2);
			// printf("\n");
			if(mode == 1){

			} else if(mode == 2){

			} else if(mode == 3){

			} else if(mode == 4){

			} else if(mode == 5){

			}
		}
	}


	return 0;
}

