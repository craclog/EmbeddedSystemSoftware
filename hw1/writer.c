#include "20131579.h"
/* 
 * Output Process 
 */

int fnd_dev, led_dev, dot_dev, lcd_dev, buz_dev;
unsigned long *fpga_addr = 0;
unsigned char *led_addr = 0;

int output_main(){
	key_t outputq_keyid;	
	output_buf o_msg; 
	
	printf("\t\tWRITER START\n");
	outputq_keyid = msgget((key_t)OUTPUTQ_KEY, IPC_CREAT|0666); /* get message queue key_id */
	if(outputq_keyid == -1){
		perror("writer::msgget error : ");
		exit(1);
	}
	if ((fnd_dev = open(FND_DEVICE, O_RDWR)) == -1) {/* open fnd device */
		printf("writer::Device open error : %s\n", FND_DEVICE);
		exit(1);
	}
	led_dev = open("/dev/mem", O_RDWR | O_SYNC); /* open led device (memory device open) */
	if (led_dev < 0) { 
		perror("/dev/mem open error");
		exit(1);
	}
	fpga_addr = (unsigned long *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, led_dev, FPGA_BASE_ADDRESS);
	if (fpga_addr == MAP_FAILED) { /* mapping fail check */
		printf("mmap error!\n");
		close(led_dev);
		exit(1);
	}
	led_addr=(unsigned char*)((void*)fpga_addr + LED_ADDR);
	if ((dot_dev = open(DOT_DEVICE, O_WRONLY)) == -1) { /* open dot device */
		printf("Device open error : %s\n",DOT_DEVICE);
		exit(1);
	}
	if ((lcd_dev = open(LCD_DEVICE, O_WRONLY)) == -1) { /* open text lcd device */
		printf("Device open error : %s\n", LCD_DEVICE);
		exit(1);
	}
	if ((buz_dev = open(BUZ_DEVICE, O_RDWR)) == -1) { /* open buzzer device */
		printf("Device open error : %s\n",BUZ_DEVICE);
		exit(1);
	}
	o_msg.mtype = OUTPUTQ_KEY;
	/* Read message from processor */
	while(1){
		if(msgrcv(outputq_keyid, (void *)&o_msg, sizeof(output_buf) - sizeof(long), 0, 0)
				== -1){
			perror("writer::msgrcv error");
			exit(1);
		}
		if(o_msg.fix_bit & FIX_FND)	write_fnd(o_msg.fnd);
		if(o_msg.fix_bit & FIX_LED)	write_led(o_msg.led);
		if(o_msg.fix_bit & FIX_LCD) write_lcd(o_msg.text);
		if(o_msg.fix_bit & FIX_DOT)	write_dot(o_msg.hex_dot);
		if(o_msg.fix_bit & FIX_BUZ)	write_buz(o_msg.buz);
		if(o_msg.fix_bit == FIX_DIE) break;
	}
	/* close all devices */
	close(fnd_dev);
	munmap(led_addr, 4096);
	close(led_dev);
	close(lcd_dev);
	close(dot_dev);
	close(buz_dev);
	printf("\t\tWRITER END\n");
	return 0;
}
/*
 * write data to fpga FND device 
 */
void write_fnd(unsigned char msg[4]){ 
	unsigned char data[4];
	int i=0;
	for(i=0; i<4; i++) data[i] = msg[i];
	if(write(fnd_dev,&data,4) == -1) {
		perror("writer::write fnd error");
 	  	exit(1);
	}
}
/*
 * write data to fpga LED device
 */
void write_led(unsigned char n){
	unsigned char data = n;
	if( (data<0) || (data>255) ) { 
		printf("\t\tInvalid range!\n");
		exit(1);
	}
	/* led data register's address : 0x08000016(0x08000000+16) */
	*led_addr = data; 
}
/*
 * write data to fpga dot device
 */
void write_dot(unsigned char data[10]){
	int size = sizeof(unsigned char)*10;
	write(dot_dev, data, size);	
}
/*
 * write data to fpga text LCD device
 */
void write_lcd(unsigned char str[MAX_STR_BUFF]){
	write(lcd_dev, str, MAX_STR_BUFF);
}
/*
 * write data to fpga buzzer device
 */
void write_buz(unsigned char data){
	write(buz_dev, &data, 1);
}

