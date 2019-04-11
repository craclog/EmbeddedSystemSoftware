#include "mque.h"

#define DEFAULT_MODE 1

int mode = DEFAULT_MODE;

int main(){
	pid_t reader_pid, writer_pid;
	key_t inputq_keyid, outputq_keyid;	
	input_buf msg;

	inputq_keyid = msgget((key_t)INPUTQ_KEY, IPC_CREAT|0666);
	if(inputq_keyid == -1){
		perror("msgget error : ");
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


	while(1){
		if(msgrcv(inputq_keyid, (void *)&msg, sizeof(input_buf), INPUTQ_KEY, 0)
				== -1){
			perror("processor::msgrcv error:");
			exit(1);
		}
		// receive function key
		if(msg.type == FUNCTION_KEY){
			printf("processor::Func_key, value:%d, ", msg.ev.code);
			if(msg.ev.value == KEY_RELEASE) 
				printf("released\n");
			else 
				printf("pressed\n");
		}
		// receive switch key
		else if(msg.type == SWITCH_KEY){
			printf("processor::Switch_key, sw_num:%d, sw_id1:%d", msg.sw_num, msg.sw_id1);
			if(msg.sw_num == 2) printf(", sw_id2:%d", msg.sw_id2);
			printf("\n");
		}
	}


	return 0;
}

