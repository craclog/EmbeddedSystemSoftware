#include "mque.h"
#include <sys/mman.h>

#define FND_DEVICE "/dev/fpga_fnd"
#define LED_DEVICE "/dev/fpga_led"

#define FPGA_BASE_ADDRESS 0x08000000
#define LED_ADDR 0x16				

int fnd_dev, led_dev;
unsigned long *fpga_addr = 0;
unsigned char *led_addr = 0;

void write_fnd(unsigned char msg[4]){
	unsigned char data[4];
	int i=0;
	for(i=0; i<4; i++){
		data[i] = msg[i];
	}
	if(write(fnd_dev,&data,4) == -1) {
		perror("writer::write fnd error");
 	  	exit(1);
	}
	// close(fnd_dev);
}
void write_led(unsigned char n){
	unsigned char data;
	if( (data<0) || (data>255) ) { 
		printf("Invalid range!\n");
		exit(1);
	}
	//led data register's address : 0x08000016(0x08000000+16)
	led_addr=(unsigned char*)((void*)fpga_addr+LED_ADDR);	
	*led_addr = data;
}

//TODO
/*
munmap;
close(...);
*/
int main(){
	key_t outputq_keyid;	
	output_buf o_msg; 
	
	int i=0;
	printf("\t\tWRITER\n");
	// get message queue key_id
	outputq_keyid = msgget((key_t)OUTPUTQ_KEY, IPC_CREAT|0666);
	if(outputq_keyid == -1){
		perror("writer::msgget error : ");
		exit(1);
	}
	// open fnd device
	if ((fnd_dev = open(FND_DEVICE, O_RDWR)) == -1) {
		printf("writer::Device open error : %s\n", FND_DEVICE);
		exit(1);
	}
	// open led device
	led_dev = open("/dev/mem", O_RDWR | O_SYNC); //memory device open
	if (led_dev < 0) {  //open fail check
		perror("/dev/mem open error");
		exit(1);
	}
	fpga_addr = (unsigned long *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, led_dev, FPGA_BASE_ADDRESS);
	if (fpga_addr == MAP_FAILED) //mapping fail check
	{
		printf("mmap error!\n");
		close(led_dev);
		exit(1);
	}

	o_msg.mtype = OUTPUTQ_KEY;
	while(1){
		if(msgrcv(outputq_keyid, (void *)&o_msg, sizeof(output_buf), OUTPUTQ_KEY, 0)
				== -1){
			perror("writer::msgrcv error:");
			exit(1);
		}
		if(o_msg.fix_bit & FIX_FND){
			write_fnd(o_msg.fnd);
		}
		if(o_msg.fix_bit & FIX_LED){
			write_led(o_msg.led);
		}
		if(o_msg.fix_bit & FIX_TEXT){
			
		}
		if(o_msg.fix_bit & FIX_DOT){
			
		}
		
	}
	return 0;
}
