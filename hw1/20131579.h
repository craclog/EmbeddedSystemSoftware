#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <linux/input.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/mman.h>

#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <semaphore.h>

/* message queue*/
#define INPUTQ_KEY 	925
#define OUTPUTQ_KEY 991

/* input_buf */
#define BUFF_SIZE 64
#define FUNCTION_KEY 	0	//readkey
#define SWITCH_KEY 		1

/* input_event */
#define KEY_RELEASE 0
#define KEY_PRESS 	1

/* output_buf */
#define FIX_FND 	1
#define FIX_LED 	2
#define FIX_TEXT 	4
#define FIX_DOT 	8

/* processor.c */
#define DEFAULT_MODE 1
#define MAX_MODE_NUM 4	// add to 5

#define BACK 158
#define VOL_UP 115
#define VOL_DOWN 114

#define MODE_CLOCK 1
#define MODE_COUNTER 2
#define MODE_TEXTEDITOR 3
#define MODE_DRAWBOARD 4

/* reader.c */
#define FUNCTION_DEVICE "/dev/input/event0"
#define SWITCH_DEVICE "/dev/fpga_push_switch"

/* writer */
#define FND_DEVICE "/dev/fpga_fnd"
#define LED_DEVICE "/dev/fpga_led"

#define FPGA_BASE_ADDRESS 0x08000000
#define LED_ADDR 0x16				

typedef struct {
	long mtype;
	int type;
	int press;
	struct input_event ev;
	int sw_num;
	unsigned char sw_id1, sw_id2;
}input_buf;

typedef struct {
	long mtype;
	int fix_bit;
	unsigned char fnd[4];	//minute
	unsigned char led;	// Binary num
	unsigned char* text; 
	int text_size;
	int dot[10];
}output_buf;


/* MODE_CLOCK */
void send_fnd(int data);
void send_led(int data);