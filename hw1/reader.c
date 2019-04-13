#include "20131579.h"

int input_main(){
	key_t inputq_keyid;
	input_buf msg;
	int rd, fd1, fd2;
	struct input_event ev;
	unsigned char switch_buf[9], delay_buf[9];
	int i;

	printf("\tREADER\n");
//	printf("sizeof ev : %d\n", (int)sizeof(struct input_event));
	if((fd1 = open(FUNCTION_DEVICE, O_RDONLY | O_NONBLOCK)) == -1){
		printf("%s is not a valid device\n", FUNCTION_DEVICE);
		perror("");
		exit(1);
	}
	if((fd2 = open(SWITCH_DEVICE, O_RDWR)) == -1){
		printf("%s is not a valid device\n", SWITCH_DEVICE);
		perror("");
		exit(1);
	}

	inputq_keyid = msgget((key_t)INPUTQ_KEY, IPC_CREAT|0666);
	if(inputq_keyid == -1){
		perror("reader::msgget error : ");
		exit(1);
	}
	printf("reader::inputq_keyid : %d\n", inputq_keyid);
	
	usleep(10000);
	msg.mtype = INPUTQ_KEY;
	while(1){
		// usleep(100000);
		msg.ev.code = 0;
		// read function key
		// if((rd = read(fd1, ev, sizeof(struct input_event)*BUFF_SIZE)) >= sizeof(struct input_event)){
		if((rd = read(fd1, (void *)&ev, sizeof(struct input_event))) >= sizeof(struct input_event)){
			msg.type = FUNCTION_KEY;
			msg.press = ev.value;
			msg.ev = ev;

			if(ev.type == 1 && msg.press == KEY_RELEASE && msg.ev.code > 100){
				printf("reader::send func\n");
				printf ("Type[%d] Value[%d] Code[%d]\n", ev.type, ev.value, (ev.code));

				if(msgsnd(inputq_keyid, (void *)&msg, sizeof(input_buf) - sizeof(long), IPC_NOWAIT)) {
					perror("reader::msgsnd error");
					exit(1);
				}
			}
		}

		// read switch key until key release
		read(fd2, delay_buf, sizeof(delay_buf));		
		for(i=0; i<9; i++){
			if(delay_buf[i] == 1) {
				while(1){
					int cnt = 0;
					read(fd2, switch_buf, sizeof(switch_buf));		
					for(i=0; i<9; i++){
						delay_buf[i] |= switch_buf[i];
						if(switch_buf[i] == 1) cnt++;
					}
					if(cnt == 0) break;
				}
				break;
			}
		}
		// get information from read
		int sw_id1, sw_id2 = -1;
		int sw_num = 0;
		for(i=0; i<9; i++){
			if(delay_buf[i] == 1) {
				if(sw_num == 0) sw_id1 = i+1;
				else if(sw_num == 1) sw_id2 = i+1;
				sw_num++;
			}
			delay_buf[i] = 0;
			switch_buf[i] = 0;
		}
		if(0 < sw_num && sw_num < 3 ){
			msg.type = SWITCH_KEY;
			msg.sw_num = sw_num;
			msg.sw_id1 = sw_id1;
			msg.sw_id2 = sw_id2;
			printf("reader::send switch\n");
			// printf("sw_id1: %d, sw_id2: %d\n", sw_id1, sw_id2);
			if(msgsnd(inputq_keyid, (void *)&msg, sizeof(input_buf) - sizeof(long), IPC_NOWAIT)) {
				perror("reader::msgsnd error: ");
				exit(1);
			}
		}
	}
	printf("\tREADER END\n");
	return 0;
}
