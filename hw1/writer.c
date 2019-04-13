#include "20131579.h"

int fnd_dev, led_dev, dot_dev, lcd_dev;
unsigned long *fpga_addr = 0;
unsigned char *led_addr = 0;

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

void write_fnd(unsigned char msg[4]){
	unsigned char data[4];
	int i=0;
	printf("writer::fnd:");
	for(i=0; i<4; i++){
		data[i] = msg[i];
		printf("%d", data[i]);
	}
	printf("\n");
	if(write(fnd_dev,&data,4) == -1) {
		perror("writer::write fnd error");
 	  	exit(1);
	}
	// close(fnd_dev);
}
void write_led(unsigned char n){
	unsigned char data = n;
	printf("writer::led:%d\n", data);
	if( (data<0) || (data>255) ) { 
		printf("Invalid range!\n");
		exit(1);
	}
	*led_addr = data; //led data register's address : 0x08000016(0x08000000+16)
}
void write_dot(int idx){
	int size = sizeof(dot_data[idx]);
	write(dot_dev,dot_data[idx],size);	
}
void write_lcd(unsigned char str[MAX_STR_BUFF]){
	write(lcd_dev, str, MAX_STR_BUFF);
}

//TODO
/*
munmap;
close(...);
*/
int output_main(){
	key_t outputq_keyid;	
	output_buf o_msg; 
	
	printf("\t\tWRITER\n");
	// get message queue key_id
	outputq_keyid = msgget((key_t)OUTPUTQ_KEY, IPC_CREAT|0666);
	if(outputq_keyid == -1){
		perror("writer::msgget error : ");
		exit(1);
	}
	printf("writer::outputq_keyid : %d\n", outputq_keyid);

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
	led_addr=(unsigned char*)((void*)fpga_addr+LED_ADDR);	
	// open dot device
	if ((dot_dev = open(DOT_DEVICE, O_WRONLY)) == -1) {
		printf("Device open error : %s\n",DOT_DEVICE);
		exit(1);
	}
	// open text lcd device
	if ((lcd_dev = open(LCD_DEVICE, O_WRONLY)) == -1) {
		printf("Device open error : %s\n", LCD_DEVICE);
		return -1;
	}
	o_msg.mtype = OUTPUTQ_KEY;
	while(1){
		if(msgrcv(outputq_keyid, (void *)&o_msg, sizeof(output_buf) - sizeof(long), 0, 0)
				== -1){
			perror("writer::msgrcv error");
			exit(1);
		}
		if(o_msg.fix_bit & FIX_FND)	write_fnd(o_msg.fnd);
		if(o_msg.fix_bit & FIX_LED)	write_led(o_msg.led);
		if(o_msg.fix_bit & FIX_LCD) write_lcd(o_msg.text);
		if(o_msg.fix_bit & FIX_DOT)	write_dot(o_msg.dot);
	}
	return 0;
}
